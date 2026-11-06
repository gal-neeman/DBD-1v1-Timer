#include "MainWindow.h"
#include <windowsx.h>
#include "Globals.h"
#include "HotkeyManager.h"
#include "ResourceUtils.h"
#include "Program.h"
#include "TimerApi/Dtos/StatusDto.h"
#include "TimerApi/Dtos/PlayerDetailsDto.h"
#include <memory>
#include <algorithm>
#include <cmath>

using Microsoft::WRL::ComPtr;

MainWindow::MainWindow() = default;

MainWindow::~MainWindow() = default;

HRESULT MainWindow::createGraphicsResources()
{
	HRESULT hr = S_OK;

	// Set up render target and brush
	if (pRenderTarget_ == nullptr)
	{
		RECT rc;
		GetClientRect(hwnd_, &rc);

		const D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		const D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_SOFTWARE,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			96.0f, 96.0f,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		);

		hr = pFactory_->CreateHwndRenderTarget(
			rtProperties,
			D2D1::HwndRenderTargetProperties(hwnd_, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
			&pRenderTarget_
		);

		if (SUCCEEDED(hr)) {
			// Extract ColorF from the palette
			const D2D1_COLOR_F timerColor = paletteToColorf(appSettings.colors.timerColor);
			const D2D1_COLOR_F selectedTimerColor = paletteToColorf(appSettings.colors.selectedTimerColor);
			const D2D1_COLOR_F lastSecondsColor = paletteToColorf(appSettings.colors.lastSecondsColor);
			backgroundColor_ = paletteToColorf(appSettings.colors.backgroundColor);

			// timer color brush
			hr = pRenderTarget_->CreateSolidColorBrush(timerColor, &pBrushTimer_);

			// selected timer color brush
			hr = pRenderTarget_->CreateSolidColorBrush(selectedTimerColor, &pBrushSelectedTimer_);

			// last seconds color brush
			hr = pRenderTarget_->CreateSolidColorBrush(lastSecondsColor, &pBrushLastSeconds_);
		}
	}

	return hr;
}

HRESULT MainWindow::createDeviceIndependentResources()
{
	HRESULT hr = S_OK;
	// Set up write factory
	if (pWriteFactory_ == nullptr)
	{
		if (pWriteFactory_ == nullptr) {
			hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory)
				, reinterpret_cast<IUnknown**>(pWriteFactory_.GetAddressOf()));
		}

		if (SUCCEEDED(hr) && pTextFormat_ == nullptr)
		{
			// Set up text format
			static constexpr int fontSize = 34;

			hr = pWriteFactory_->CreateTextFormat(
				FONT_NAME_TIMER,
				nullptr,
				DWRITE_FONT_WEIGHT_BOLD,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_EXTRA_EXPANDED,
				fontSize,
				L"",
				&pTextFormat_
			);

			if (SUCCEEDED(hr))
			{
				// Center the text horizontally and vertically.
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

				if (SUCCEEDED(hr))
				{
					hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
				}
			}
		}
	}

	return hr;
}

void MainWindow::discardGraphicsResources()
{
	pFactory_.Reset();
	pRenderTarget_.Reset();
	pBrushTimer_.Reset();
	pBrushSelectedTimer_.Reset();
	pBrushLastSeconds_.Reset();
	pWriteFactory_.Reset();
	pTextFormat_.Reset();
}

LRESULT MainWindow::hitTest(const LPARAM lParam) const
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ScreenToClient(hwnd_, &pt);

	RECT rc;
	GetClientRect(hwnd_, &rc);

	const bool left = pt.x < RESIZE_GRIP;
	const bool right = pt.x >= rc.right - RESIZE_GRIP;
	const bool top = pt.y < RESIZE_GRIP;
	const bool bottom = pt.y >= rc.bottom - RESIZE_GRIP;

	// Only the corners resize, so the window scales diagonally
	if (top && left) return HTTOPLEFT;
	if (top && right) return HTTOPRIGHT;
	if (bottom && left) return HTBOTTOMLEFT;
	if (bottom && right) return HTBOTTOMRIGHT;

	return HTCLIENT; // dragged from WM_LBUTTONDOWN
}

void MainWindow::constrainSizingRect(const WPARAM edge, RECT* rect) const
{
	const float scaleX = static_cast<float>(rect->right - rect->left) / SIZE_MAIN_WIDTH;
	const float scaleY = static_cast<float>(rect->bottom - rect->top) / SIZE_MAIN_HEIGHT;
	const float scale = std::clamp((std::max)(scaleX, scaleY), MIN_SCALE, MAX_SCALE);

	const LONG width = std::lround(SIZE_MAIN_WIDTH * scale);
	const LONG height = std::lround(SIZE_MAIN_HEIGHT * scale);

	// Keep the corner opposite to the dragged one in place
	if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT || edge == WMSZ_BOTTOMLEFT)
		rect->left = rect->right - width;
	else
		rect->right = rect->left + width;

	if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
		rect->top = rect->bottom - height;
	else
		rect->bottom = rect->top + height;
}

void MainWindow::handleResize(const UINT width, const UINT height)
{
	float fWidth = static_cast<float>(width);
	float fHeight = static_cast<float>(height);
	winSize_[0] = fWidth;
	winSize_[1] = fHeight;
	scale_ = (std::min)(fWidth / SIZE_MAIN_WIDTH, fHeight / SIZE_MAIN_HEIGHT);

	// WM_SIZE can arrive before the render target exists
	if (pRenderTarget_ != nullptr) {
		pRenderTarget_->Resize(D2D1::SizeU(width, height));
	}

	requestRedraw();
}

void MainWindow::handlePainting()
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd_, &ps);
	pRenderTarget_->BeginDraw();

	// workaround to visible edges issue while transparent
	if (appSettings.optionTransparent) {
		pRenderTarget_->Clear(D2D1::ColorF(0, 0, 0));
	}
	else {
		pRenderTarget_->Clear(backgroundColor_);
	}

	// Lay out in base units and scale everything to the window size
	pRenderTarget_->SetTransform(D2D1::Matrix3x2F::Scale(scale_, scale_));

	constexpr float halfWidth = SIZE_MAIN_WIDTH / 2.0f;
	const D2D1_RECT_F rect1 = D2D1::RectF(0, 0, halfWidth, SIZE_MAIN_HEIGHT);
	const D2D1_RECT_F rect2 = D2D1::RectF(halfWidth, 0, SIZE_MAIN_WIDTH, SIZE_MAIN_HEIGHT);

	if (pWriteFactory_ != nullptr)
	{
		if (activeTimer_ != nullptr)
		{
			// Select color for timer 2
			ID2D1SolidColorBrush* pBrushTimer_2 = getTimer2Brush();

			// draw timers
			if (activeTimer_ == &timer1) {
				timer1.draw(pRenderTarget_.Get(), pTextFormat_.Get(), rect1, pBrushSelectedTimer_.Get());
			}
			else {
				timer1.draw(pRenderTarget_.Get(), pTextFormat_.Get(), rect1, pBrushTimer_.Get());
			}
			timer2.draw(pRenderTarget_.Get(), pTextFormat_.Get(), rect2, pBrushTimer_2);
		}
		else
		{
			timer1.draw(pRenderTarget_.Get(), pTextFormat_.Get(), rect1, pBrushTimer_.Get());
			timer2.draw(pRenderTarget_.Get(), pTextFormat_.Get(), rect2, pBrushTimer_.Get());
		}
	}

	const HRESULT hr = pRenderTarget_->EndDraw();

	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
	{
		discardGraphicsResources();
	}
	EndPaint(hwnd_, &ps);
}

D2D1_COLOR_F MainWindow::paletteToColorf(const int colorIndex) {
	const COLORREF color = PALETTE[colorIndex];
	return D2D1::ColorF(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
}

void MainWindow::refreshBrushes()
{
	// timer color
	pBrushTimer_->SetColor(paletteToColorf(appSettings.colors.timerColor));

	// selected timer color
	pBrushSelectedTimer_->SetColor(paletteToColorf(appSettings.colors.selectedTimerColor));

	// last seconds color
	pBrushLastSeconds_->SetColor(paletteToColorf(appSettings.colors.lastSecondsColor));

	// background color
	backgroundColor_ = paletteToColorf(appSettings.colors.backgroundColor);
}

LRESULT MainWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
{
	try
	{
		switch (wMsg)
		{
		case WM_CREATE:
		{
			if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, pFactory_.GetAddressOf()))) {
				return -1;
			}
			createGraphicsResources();
			createDeviceIndependentResources();

			SetTimer(hwnd_, 0, 16, nullptr);
			return 0;
		}
		case WM_CLOSE:
		{
			exitApp();
			return 0;
		}
		case WM_NCHITTEST:
			return hitTest(lParam);
		case WM_LBUTTONDOWN:
			// Start the native window move loop, as if the (nonexistent) caption was grabbed
			SendMessage(hwnd_, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
			return 0;
		case WM_SIZING:
			constrainSizingRect(wParam, reinterpret_cast<RECT*>(lParam));
			return TRUE;
		case WM_SIZE:
			handleResize(LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_COMMAND:
		{
			switch (wParam)
			{
			case MENU_SETTINGS:
				if (settingsWindow.window() == nullptr) // dont create multiple settings windows
				{
					// Create and show settings window
					if (!settingsWindow.create(SETTINGS_WINDOW_TITLE, 500, 200, SIZE_SETTINGS_WIDTH, SIZE_SETTINGS_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, hwnd_, nullptr	, nullptr, nullptr)) {
						return 0;
					}

					ShowWindow(settingsWindow.window(), SW_SHOW);
				}
				else
				{
					SetForegroundWindow(settingsWindow.window());
				}
				return 0;
			case MENU_STATISTICS:
				if (statisticsWindow.window() == nullptr) // dont create multiple statistics windows
				{
					// Create and show statistics window
					if (!statisticsWindow.create(STATISTICS_WINDOW_TITLE, 500, 200, SIZE_STATISTICS_WIDTH, SIZE_STATISTICS_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, hwnd_, nullptr, nullptr, nullptr)) {
						return 0;
					}

					ShowWindow(statisticsWindow.window(), SW_SHOW);
				}
				else
				{
					SetForegroundWindow(statisticsWindow.window());
				}
				return 0;
			case MENU_QUIT:
				exitApp();
				return 0;
			default:
				break;
			}
			return 0;
		}
		case WM_CONTEXTMENU:
		{
			// mouse pos
			const int mouseX = GET_X_LPARAM(lParam);
			const int mouseY = GET_Y_LPARAM(lParam);

			// create popup menu
			HMENU hMenu = CreatePopupMenu();
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, MENU_QUIT, L"Quit");
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 100, L"");
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, MENU_STATISTICS, L"DBDLeague Stats");
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, MENU_SETTINGS, L"Settings");
			SetForegroundWindow(hwnd_);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, mouseX, mouseY, 0, hwnd_, nullptr);
			DestroyMenu(hMenu);
			return 0;
		}
		case WM_PAINT:
		{
			handlePainting();
			return 0;
		}
		case REFRESH_BRUSHES:
			refreshBrushes();
			requestRedraw();
			break;
		case HOTKEY_HIT:
		{
			const int key = (int)wParam;
			handleHotKey(key);
			requestRedraw();
		}
			break;
		case CONTROLLER_INPUT:
			handleControllerInput(wParam);
			break;
		case TIMER_API_STATUS_RESPONSE:
		{
			const std::unique_ptr<StatusDto> status(reinterpret_cast<StatusDto*>(lParam));
			break;
		}
		case TIMER_API_PLAYER_RESPONSE:
		{
			const std::unique_ptr<PlayerDetailsDto> playerDetails(reinterpret_cast<PlayerDetailsDto*>(lParam));
			break;
		}
		case WM_TIMER:
		{
			if (timer1.isRunning() || timer2.isRunning()) {
				timer1.updateTime();
				timer2.updateTime();
				requestRedraw();
			}
			break;
		}
		default:
			break;
		}
	}
	catch (const std::exception &e) // End the program in case of exceptions
	{
		exitApp();
	}

	return DefWindowProc(window(), wMsg, wParam, lParam);
}

void MainWindow::applyWindowSettings(const SettingsStruct& settings) const
{
	// transparency
	if (settings.optionTransparent) { // add transparent effect
		SetLayeredWindowAttributes(hwnd_, 0, 0, LWA_COLORKEY);
	}
	else { // remove transparent effect
		SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
	}

	// click through
	if (settings.optionClickThrough) { // make click through
		// Get the current window style
		LONG style = GetWindowLong(hwnd_, GWL_EXSTYLE);

		// Add the new style to the current styles
		style |= WS_EX_TRANSPARENT;

		// Set the new style
		SetWindowLong(hwnd_, GWL_EXSTYLE, style);
	}
}

void MainWindow::handleHotKey(const int code)
{
	bool isActivateTimer = appSettings.optionStartOnChange;

	switch (code)
	{
	case KEY_TIMER1: // timer1 key
		activeTimer_ = &timer1;
		break;
	case KEY_TIMER2: // timer2 key
		activeTimer_ = &timer2;
		break;
	case KEY_START: // start key
		isActivateTimer = true;
		break;
	case KEY_START_NO_RESET: // start no reset key
	{
		if (activeTimer_ == nullptr) return;

		if (activeTimer_->getTimerState() == TimerState::Running)
			activeTimer_->stopTimer();
		else
			activeTimer_->startTimer();

		return; // Skip the start / stop / reset below, which would act on the timer a second time
	}
	default:
		break;
	}

	if (isActivateTimer && activeTimer_ != nullptr)
	{
		if (activeTimer_->getTimerState() == TimerState::Zero) {
			activeTimer_->startTimer();
		}
		else if (activeTimer_->getTimerState() == TimerState::Running) {
			activeTimer_->stopTimer();
		}
		else {
			activeTimer_->resetTimer();
		}
	}
}

void MainWindow::handleControllerInput(const WORD buttons) const
{
	if (settingsWindow.window() != nullptr)
	{
		SendMessage(settingsWindow.window(), CONTROLLER_INPUT, buttons, NULL);
		return;
	}

	HotkeyManager::execute(buttons);
}

ID2D1SolidColorBrush* MainWindow::getTimer2Brush() const {
	int lastSecondsTime = appSettings.optionLastSecondsTime * 1000;
	if (timer1.getTimeInMillis() > 0
		&& timer1.getTimeInMillis() - timer2.getTimeInMillis() <= lastSecondsTime
		&& (timer2.getTimerState() == TimerState::Running || timer2.getTimerState() == TimerState::Paused)
		&& timer1.getTimeInMillis() - timer2.getTimeInMillis() > 0)
	{
		return pBrushLastSeconds_.Get();
	}
	else if (activeTimer_ == &timer2) {
		return pBrushSelectedTimer_.Get();
	}
	else {
		return pBrushTimer_.Get();
	}
}

void MainWindow::requestRedraw() const {
	InvalidateRect(hwnd_, nullptr, FALSE);
}