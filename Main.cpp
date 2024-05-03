#include <windows.h>
#include <d2d1.h>
#include <string>
#include <wchar.h>
#include <thread>
#include <dwrite.h>
#include <windowsx.h>
#include <CommCtrl.h>

#pragma comment (lib, "d2d1")

using std::thread; using std::wstring;

// Enum assignment for readable code
enum TimerState
{
	running,
	paused,
	zero
};

// A timer struct with all the neccessary functionalities of the timer
class Timer
{
private:
	TimerState timerState;
	int time = 0;
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
		if (t1.wMilliseconds < t2.wMilliseconds) {
			seconds++;
		}

		return seconds > 0 ? seconds - 1: 0;
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

	int GetTimeInSeconds()
	{
		return time;
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

// Template class for specific window's classes to inherit from
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

// Release pointers safely
template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// The class responsible for the main window of the app
class MainWindow : public BaseWindow<MainWindow>
{
	// Resources
	ID2D1Factory* pFactory;
	ID2D1HwndRenderTarget* pRenderTarget = 0;
	ID2D1SolidColorBrush* pBrushGreen;
	ID2D1SolidColorBrush* pBrushBlue;
	ID2D1SolidColorBrush* pBrushRed;

	// Writing Resources
	IDWriteFactory* pWriteFactory;
	IDWriteTextFormat* pTextFormat;

private:
	Timer* activeTimer;

	BOOL mouseDown = false;
	int clickMousePos[2] = {0, 0};

	HRESULT CreateGraphicsResources()
	{
		HRESULT hr = S_OK;
		
		// Set up render target and brush
		if (pRenderTarget == NULL)
		{
			RECT rc;
			GetClientRect(m_hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

			hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);

			if (SUCCEEDED(hr)) {
				const D2D1_COLOR_F greenColor = D2D1::ColorF(0.0f, 1, 0.4f);
				hr = pRenderTarget->CreateSolidColorBrush(greenColor, &pBrushGreen);

				const D2D1_COLOR_F blueColor = D2D1::ColorF(0.0f, 0.6f, 1);
				hr = pRenderTarget->CreateSolidColorBrush(blueColor, &pBrushBlue);

				const D2D1_COLOR_F redColor = D2D1::ColorF(1, 0.3f, 0.3f);
				hr = pRenderTarget->CreateSolidColorBrush(redColor, &pBrushRed);
			}
		}

		return hr;
	}

	HRESULT CreateDeviceIndependentResources()
	{
		HRESULT hr = S_OK;
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
		SafeRelease(&pBrushGreen);
		SafeRelease(&pBrushBlue);
		SafeRelease(&pBrushRed);
		SafeRelease(&pWriteFactory);
		SafeRelease(&pTextFormat);
	}

	void HandlePainting()
	{
		HRESULT hrGraphics = CreateGraphicsResources();
		HRESULT hrWrite = CreateDeviceIndependentResources();

		if (SUCCEEDED(S_OK))
		{
			PAINTSTRUCT ps;
			BeginPaint(m_hwnd, &ps);
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

			D2D1_RECT_F rect1 = D2D1::RectF(0, 0, winSize[0] / 2, winSize[1]);
			D2D1_RECT_F rect2 = D2D1::RectF(winSize[0] / 2, 0, winSize[0], winSize[1]);

			if (SUCCEEDED(hrWrite))
			{
				if (activeTimer != NULL)
				{
					// Select color for timer 2
					ID2D1SolidColorBrush* pBrushTimer2;
					if (timer1.GetTimeInSeconds() > 0
						&& timer1.GetTimeInSeconds() - timer2.GetTimeInSeconds() <= 20
						&& (timer2.GetTimerState() == TimerState::running || timer2.GetTimerState() == TimerState::paused)
						&& timer1.GetTimeInSeconds() - timer2.GetTimeInSeconds() > 0)
					{
						pBrushTimer2 = pBrushRed;
					}
					else if (activeTimer == &timer2) {
						pBrushTimer2 = pBrushBlue;
					}
					else {
						pBrushTimer2 = pBrushGreen;
					}

					// Draw timers
					if (activeTimer == &timer1) {
						timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushBlue);
					}
					else {
						timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushGreen);
					}
					timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushTimer2);
				}
				else
				{
					timer1.Draw(pRenderTarget, pTextFormat, rect1, pBrushGreen);
					timer2.Draw(pRenderTarget, pTextFormat, rect2, pBrushGreen);
				}
			}

			hrGraphics = pRenderTarget->EndDraw();

			if (FAILED(hrGraphics) || hrGraphics == D2DERR_RECREATE_TARGET)
			{
				DiscardGraphicsResources();
			}
			EndPaint(m_hwnd, &ps);
		}
	}

	void HandleWindowDrag(LPARAM lParam)
	{
		RECT windowPos;
		GetWindowRect(m_hwnd, &windowPos);
		int currPos[2] = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		int xToMove = windowPos.left + (currPos[0] - clickMousePos[0]);
		int yToMove = windowPos.top + (currPos[1] - clickMousePos[1]);

		SetWindowPos(m_hwnd, NULL, xToMove, yToMove, winSize[0], winSize[1], 0);
	}

public:
	Timer timer1 = Timer();
	Timer timer2 = Timer();

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
		case WM_LBUTTONDOWN:
			mouseDown = true;
			clickMousePos[0] = GET_X_LPARAM(lParam);
			clickMousePos[1] = GET_Y_LPARAM(lParam);
			return 0;
		case WM_LBUTTONUP:
			mouseDown = false;
			return 0;
		case WM_MOUSEMOVE:
			if (mouseDown)
			{
				HandleWindowDrag(lParam);
			}
			return 0;
		case WM_COMMAND:
			switch (wParam)
			{
			case 1: // settings
				// settings logic...
				return 0;
			case 0: // Quit
				PostQuitMessage(0);
				appRunning = false;
				return 0;
			}
			return 0;
		case WM_CONTEXTMENU:
			{
				// mouse pos
				int mouseX = GET_X_LPARAM(lParam);
				int mouseY = GET_Y_LPARAM(lParam);

				// create popup menu
				HMENU hMenu = CreatePopupMenu();
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 0, L"Quit");
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 100, L"");
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 1, L"Settings");
				SetForegroundWindow(m_hwnd);
				TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, mouseX, mouseY, 0, m_hwnd, NULL);
				return 0;
			}
		}
		return DefWindowProc(Window(), wMsg, wParam, lParam);
	}
	void HandleHotKey(int code)
	{
		switch (code)
		{
		case 0: // f1
			activeTimer = &timer1;
			break;
		case 1: // f2
			activeTimer = &timer2;
			break;
		case 2: // f
			if (activeTimer != NULL)
			{
				if (activeTimer->GetTimerState() == TimerState::zero) {
					activeTimer->StartTimer();
				}
				else if (activeTimer->GetTimerState() == TimerState::running) {
					activeTimer->StopTimer();
				}
				else {
					activeTimer->ResetTimer();
				}
			}
			break;
		}
	}

	void Draw() {
		HandlePainting();
	}
};

// App Loop logic
void AppLoop(MainWindow* win)
{
	while (win->appRunning)
	{
		Sleep(1);
		win->timer1.UpdateTime();
		win->timer2.UpdateTime();
		win->Draw();
	}
}

// Global reference to the main window class instance (for the hook proc)
MainWindow* pGlobalTimerWindow = NULL;

// The hook procedure to listen for kestrokes f1, f2 and f
LRESULT CALLBACK KBHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_KEYUP && pGlobalTimerWindow != NULL)
	{
		KBDLLHOOKSTRUCT* pKbdHookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		switch (pKbdHookStruct->vkCode)
		{
		case 0x46:
			pGlobalTimerWindow->HandleHotKey(2);
			break;
		case VK_F2:
			pGlobalTimerWindow->HandleHotKey(1);
			break;
		case VK_F1:
			pGlobalTimerWindow->HandleHotKey(0);
			break;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// The main function
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create a window
	MainWindow win;
	pGlobalTimerWindow = &win;

	if (!win.Create(L"Timer", 0, 0, 275, 40, WS_EX_TOPMOST, WS_POPUP)) {
		return 0;
	}
	
	ShowWindow(win.Window(), nCmdShow);

	// Listen for keys: F1, F2, F While running in the background - Install a hook procedure
	HHOOK kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KBHook, 0, 0);

	// Create a thread for the app loop (ticks)
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