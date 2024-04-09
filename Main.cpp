#include <windows.h>
#include <d2d1.h>
#include <string>
#include <wchar.h>
#include <thread>
#include <dwrite.h>

#pragma comment (lib, "d2d1")

using std::thread; using std::wstring;

enum TimerState
{
	running,
	paused,
	zero
};

class Timer
{
private:
	TimerState timerState;
	int time;
	SYSTEMTIME startTime;
	SYSTEMTIME updatingTime;

	int SubtractTimes(SYSTEMTIME t1, SYSTEMTIME t2)
	{
		int seconds = 0;
		int minutes = 0;

		if (t1.wMinute > t2.wMinute)
		{
			minutes = (60 - t1.wMinute) + t2.wMinute;
		}
		else
		{
			minutes = t2.wMinute - t1.wMinute;
		}
		seconds = t2.wSecond - t1.wSecond;
		seconds = (60 * minutes) + seconds;

		return seconds;
	}

	wstring GetTimeAsText()
	{
		int minutesInt = 0;
		int secondsInt = time;

		while (secondsInt > 59) {
			minutesInt++;
			secondsInt -= 60;
		}

		wstring secondsStr = std::to_wstring(secondsInt);
		wstring minutesStr = std::to_wstring(minutesInt);

		wstring text = L"00:00";
		if (minutesInt > 9) {
			text[0] = minutesStr[0];
			text[1] = minutesStr[1];
		}
		else {
			text[1] = minutesStr[0];
		}

		if (secondsInt > 9)
		{
			text[3] = secondsStr[0];
			text[4] = secondsStr[1];
		}
		else {
			text[4] = secondsStr[0];
		}

		return text;
	}

public:
	Timer() {
		time = 0;
		timerState = TimerState::zero;
	}

	TimerState GetTimerState()
	{
		return timerState;
	}

	void StartTimer()
	{
		timerState = TimerState::running;
		GetSystemTime(&startTime);
	}

	void StopTimer()
	{
		timerState = TimerState::paused;
	}

	void ResetTimer()
	{
		timerState = TimerState::zero;
		GetLocalTime(&startTime);
		GetLocalTime(&updatingTime);
		time = SubtractTimes(startTime, updatingTime);
	}

	void UpdateTime()
	{
		if (timerState == TimerState::running)
		{
			GetSystemTime(&updatingTime);
			time = SubtractTimes(startTime, updatingTime);
		}
	}

	void Draw(ID2D1HwndRenderTarget* pRenderTarget, IDWriteTextFormat* pTextFormat, D2D1_RECT_F rectF, ID2D1SolidColorBrush* pBrush) // Only call from within an active render target begin draw scope
	{
		if (pRenderTarget != NULL)
		{
			wstring timeStr = GetTimeAsText();
			const WCHAR* timeText = timeStr.c_str();

			pRenderTarget->DrawTextW(
				timeText,
				5,
				pTextFormat,
				rectF,
				pBrush);
		}
	}
};

template <class DERIVED_CLASS> class BaseWindow
{
public:
	BaseWindow() : m_hwnd(NULL) { };

	HWND Window() const { return m_hwnd; }

	bool appRunning = false;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_CLASS* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* cStruct = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_CLASS*)cStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_CLASS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis != NULL)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	BOOL Create(
		LPCWSTR lpWindowName,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int width = CW_USEDEFAULT,
		int height = CW_USEDEFAULT,
		DWORD dwExStyle = WS_OVERLAPPEDWINDOW,
		DWORD dwStyle = 0,
		HWND parent = NULL,
		HMENU hMenu = NULL,
		HINSTANCE hInstance = NULL,
		LPVOID lpParam = 0
	) {
		// create a window class
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = DERIVED_CLASS::WindowProc;
		wc.lpszClassName = ClassName();
		wc.hInstance = NULL;
		wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(dwExStyle, ClassName(), lpWindowName, dwStyle, x, y, width, height, parent, hMenu, hInstance, this);

		winSize[0] = width; winSize[1] = height;

		return (m_hwnd ? TRUE : FALSE);
	}

protected:
	HWND m_hwnd;
	int winSize[2] = { 0, 0 };

	virtual LPCWSTR ClassName() const { return 0; }
	virtual LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam) = 0;
};

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class MainWindow : public BaseWindow<MainWindow>
{
	// Resources
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pBrush;

	// Writing Resources
	IDWriteFactory* pWriteFactory;
	IDWriteTextFormat* pTextFormat;

private:
	HRESULT CreateGraphicsResources()
	{
		HRESULT hr = S_OK;

		// Set up render target and brush
		if (pRenderTarget == NULL);
		{
			RECT rc;
			GetClientRect(m_hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

			D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(),
				0,
				0,
				D2D1_RENDER_TARGET_USAGE_NONE,
				D2D1_FEATURE_LEVEL_DEFAULT
			);

			hr = pFactory->CreateHwndRenderTarget(properties, D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);

			if (SUCCEEDED(hr)) {
				const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 0.7f, 0);
				hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
			}
		}

		// Set up write factory
		if (pWriteFactory == NULL)
		{
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory)
				, reinterpret_cast<IUnknown**>(&pWriteFactory));

			// Set up text format
			static const WCHAR fontName[] = L"Verdana";
			static const FLOAT fontSize = 35;

			if (SUCCEEDED(hr))
			{
				pWriteFactory->CreateTextFormat(
					fontName,
					NULL,
					DWRITE_FONT_WEIGHT_NORMAL,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					fontSize,
					L"",
					&pTextFormat
				);

				if (SUCCEEDED(hr))
				{
					// Center the text horizontally and vertically.
					pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

					pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				}
			}
		}

		return hr;
	}

	void DiscardGraphicsResources()
	{
		SafeRelease(&pRenderTarget);
		SafeRelease(&pBrush);
		SafeRelease(&pWriteFactory);
		SafeRelease(&pTextFormat);
	}

	void HandlePainting()
	{
		HRESULT hr = CreateGraphicsResources();

		if (SUCCEEDED(hr))
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hwnd, &ps);
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

			D2D1_RECT_F rect1 = D2D1::RectF(0, 0, winSize[0] / 2, winSize[1]);
			timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrush);

			D2D1_RECT_F rect2 = D2D1::RectF(winSize[0] / 2, 0, winSize[0], winSize[1]);
			timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrush);

			hr = pRenderTarget->EndDraw();

			if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
			{
				DiscardGraphicsResources();
			}
			EndPaint(m_hwnd, &ps);
		}
	}

public:
	LPCWSTR ClassName() const { return L"Main Window"; }
	LRESULT HandleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (wMsg)
		{
		case WM_CREATE:
			if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory))) {
				return -1;
			}
			appRunning = true;
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			appRunning = false;
			return 0;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_F1:
				if (timer1.GetTimerState() == TimerState::zero) {
					timer1.StartTimer();
				}
				else if (timer1.GetTimerState() == TimerState::running) {
					timer1.StopTimer();
				}
				else {
					timer1.ResetTimer();
				}
				break;
			case VK_F2:
				if (timer2.GetTimerState() == TimerState::zero) {
					timer2.StartTimer();
				}
				else if (timer2.GetTimerState() == TimerState::running) {
					timer2.StopTimer();
				}
				else {
					timer2.ResetTimer();
				}
			}
			return 0;
		case WM_WINDOWPOSCHANGED:
			return 0;
		}

		return DefWindowProc(Window(), wMsg, wParam, lParam);
	}
	Timer timer1 = Timer();
	Timer timer2 = Timer();

	void Draw() {
		HandlePainting();
	}
};

void AppLoop(MainWindow* win)
{
	while (win->appRunning)
	{
		Sleep(1);
		win->timer1.UpdateTime(); // only happens if the timer is running
		win->timer2.UpdateTime(); // only happens if the timer is running
		win->Draw();
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create a window
	MainWindow win;

	if (!win.Create(L"Timer", 0, 0, 300, 80, WS_EX_TOPMOST, WS_POPUP)) {
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	thread t1(AppLoop, &win);

	// Handle messages
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	t1.join();
	return 0;
}