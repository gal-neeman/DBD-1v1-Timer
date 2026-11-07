#include "MainWindow.h"
#include <windowsx.h>
#include "Globals.h"
#include "HotkeyManager.h"
#include "ResourceUtils.h"
#include "Program.h"
#include "StatisticsWindowStrings.h"
#include "TimerApi/Dtos/StatusDto.h"
#include "TimerApi/Dtos/PlayerDetailsDto.h"
#include "TimerApi/TimerApiMessages.h"
#include "TimerApi/TimerApiService.h"
#include "League/LeagueMappers.h"
#include <memory>
#include <algorithm>
#include <cmath>

using Microsoft::WRL::ComPtr;

namespace
{
	// L"#512", or L"#?" when unranked
	std::wstring formatRank(const std::optional<int> rank)
	{
		return rank ? L"#" + std::to_wstring(*rank) : L"#?";
	}
}

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

			// overlay brush, recolored before each use
			hr = pRenderTarget_->CreateSolidColorBrush(colorRefToColorf(OVERLAY_PALETTE[OVERLAY_TEXT]), &pBrushOverlay_);
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

	// Overlay text: centered on one line in its rect
	const auto createOverlayFormat = [this](const float size, Microsoft::WRL::ComPtr<IDWriteTextFormat>& format)
	{
		if (format != nullptr) return S_OK;
		HRESULT formatHr = pWriteFactory_->CreateTextFormat(FONT_NAME_OVERLAY, nullptr, DWRITE_FONT_WEIGHT_BOLD,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"", &format);
		if (SUCCEEDED(formatHr)) formatHr = format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		if (SUCCEEDED(formatHr)) formatHr = format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if (SUCCEEDED(formatHr)) formatHr = format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		return formatHr;
	};

	if (SUCCEEDED(hr)) hr = createOverlayFormat(NAMES_FONT_SIZE, pNamesFormat_);
	if (SUCCEEDED(hr)) hr = createOverlayFormat(BAR_FONT_SIZE, pBarFormat_);
	if (SUCCEEDED(hr)) hr = createOverlayFormat(BADGE_FONT_SIZE, pBadgeFormat_);
	if (SUCCEEDED(hr) && pEllipsis_ == nullptr)
	{
		hr = pWriteFactory_->CreateEllipsisTrimmingSign(pNamesFormat_.Get(), &pEllipsis_);
	}

	return hr;
}

void MainWindow::discardGraphicsResources()
{
	// Only what depends on the render target; handlePainting recreates it on the next paint
	pRenderTarget_.Reset();
	pBrushTimer_.Reset();
	pBrushSelectedTimer_.Reset();
	pBrushLastSeconds_.Reset();
	pBrushOverlay_.Reset();
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
	const D2D1_SIZE_F base = baseSize(league_.layout);
	const float scaleX = static_cast<float>(rect->right - rect->left) / base.width;
	const float scaleY = static_cast<float>(rect->bottom - rect->top) / base.height;
	const float scale = std::clamp((std::max)(scaleX, scaleY), MIN_SCALE, MAX_SCALE);

	const LONG width = std::lround(base.width * scale);
	const LONG height = std::lround(base.height * scale);

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
	winSize_[0] = static_cast<int>(width);
	winSize_[1] = static_cast<int>(height);

	const float fWidth = static_cast<float>(width);
	const float fHeight = static_cast<float>(height);
	const D2D1_SIZE_F base = baseSize(league_.layout);
	scale_ = (std::min)(fWidth / base.width, fHeight / base.height);

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

	// Recreates the render target after a D2DERR_RECREATE_TARGET discarded it
	if (FAILED(createGraphicsResources()))
	{
		EndPaint(hwnd_, &ps);
		return;
	}

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

	if (pWriteFactory_ != nullptr)
	{
		const OverlayStats* stats = league_.overlayStats ? &*league_.overlayStats : nullptr;
		const OverlayLayout layout = stats != nullptr ? league_.layout : OverlayLayout::TimerOnly;

		switch (layout)
		{
		case OverlayLayout::Stats:
		{
			constexpr float timersWidth = OVERLAY_WIDTH - BADGE_WIDTH;
			paintTimers(D2D1::RectF(0, 0, timersWidth, TIMER_ROW_HEIGHT));
			paintRankBadge(D2D1::RectF(timersWidth, 0, OVERLAY_WIDTH, TIMER_ROW_HEIGHT), *stats);
			paintStatsBar(D2D1::RectF(0, TIMER_ROW_HEIGHT, OVERLAY_WIDTH, TIMER_ROW_HEIGHT + STATS_BAR_HEIGHT), *stats);
			break;
		}
		case OverlayLayout::StatsWithNames:
		{
			constexpr float timersBottom = NAMES_ROW_HEIGHT + TIMER_ROW_HEIGHT;
			paintNamesRow(D2D1::RectF(0, 0, OVERLAY_WIDTH, NAMES_ROW_HEIGHT), *stats);
			paintTimers(D2D1::RectF(0, NAMES_ROW_HEIGHT, OVERLAY_WIDTH, timersBottom));
			paintStatsBar(D2D1::RectF(0, timersBottom, OVERLAY_WIDTH, timersBottom + STATS_BAR_HEIGHT), *stats);
			break;
		}
		default:
			paintTimers(D2D1::RectF(0, 0, SIZE_MAIN_WIDTH, SIZE_MAIN_HEIGHT));
			break;
		}
	}

	const HRESULT hr = pRenderTarget_->EndDraw();

	if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
	{
		discardGraphicsResources();
	}
	EndPaint(hwnd_, &ps);
}

D2D1_SIZE_F MainWindow::baseSize(const OverlayLayout layout)
{
	switch (layout)
	{
	case OverlayLayout::Stats:
		return D2D1::SizeF(OVERLAY_WIDTH, TIMER_ROW_HEIGHT + STATS_BAR_HEIGHT);
	case OverlayLayout::StatsWithNames:
		return D2D1::SizeF(OVERLAY_WIDTH, NAMES_ROW_HEIGHT + TIMER_ROW_HEIGHT + STATS_BAR_HEIGHT);
	default:
		return D2D1::SizeF(SIZE_MAIN_WIDTH, SIZE_MAIN_HEIGHT);
	}
}

OverlayLayout MainWindow::derivedLayout() const
{
	if (!league_.overlayStats || !appSettings.league.statsOverlayEnabled) return OverlayLayout::TimerOnly;
	return appSettings.league.showNamesAndScore ? OverlayLayout::StatsWithNames : OverlayLayout::Stats;
}

void MainWindow::setLayout(const OverlayLayout layout)
{
	// The move/size loop ends by restoring the size it started with, which would undo the resize below.
	// WM_EXITSIZEMOVE applies the layout instead.
	if (layout == league_.layout || inSizeMove_) return;
	league_.layout = layout;

	// WM_SIZE follows synchronously and recomputes scale_ against the new base size
	const D2D1_SIZE_F base = baseSize(layout);
	SetWindowPos(hwnd_, nullptr, 0, 0, std::lround(base.width * scale_), std::lround(base.height * scale_),
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void MainWindow::paintTimers(const D2D1_RECT_F& row)
{
	const float middle = (row.left + row.right) / 2.0f;
	const D2D1_RECT_F rect1 = D2D1::RectF(row.left, row.top, middle, row.bottom);
	const D2D1_RECT_F rect2 = D2D1::RectF(middle, row.top, row.right, row.bottom);

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

void MainWindow::paintRankBadge(const D2D1_RECT_F& rect, const OverlayStats& stats)
{
	fillRect(rect, OVERLAY_BADGE_BACKGROUND);
	drawText(formatRank(stats.playerRank), pBadgeFormat_.Get(), rect, OVERLAY_TEXT);
}

void MainWindow::paintNamesRow(const D2D1_RECT_F& rect, const OverlayStats& stats)
{
	// Equal name cells on both sides keep the score centered over the gap between the timers
	const float nameWidth = (rect.right - rect.left - SCORE_WIDTH) / 2.0f;
	const D2D1_RECT_F playerCell = D2D1::RectF(rect.left, rect.top, rect.left + nameWidth, rect.bottom);
	const D2D1_RECT_F scoreCell = D2D1::RectF(playerCell.right, rect.top, playerCell.right + SCORE_WIDTH, rect.bottom);
	const D2D1_RECT_F opponentCell = D2D1::RectF(scoreCell.right, rect.top, rect.right, rect.bottom);

	fillRect(playerCell, OVERLAY_NAME_BACKGROUND);
	fillRect(scoreCell, OVERLAY_SCORE_BACKGROUND);
	fillRect(opponentCell, OVERLAY_NAME_BACKGROUND);

	drawNameWithRank(playerCell, stats.playerName, stats.playerRank);
	if (stats.opponentName) drawNameWithRank(opponentCell, *stats.opponentName, stats.opponentRank); // empty outside a match

	const std::wstring score = std::to_wstring(stats.scorePlayer) + L" - " + std::to_wstring(stats.scoreOpponent);
	drawText(score, pNamesFormat_.Get(), scoreCell, OVERLAY_TEXT);
}

void MainWindow::paintStatsBar(const D2D1_RECT_F& rect, const OverlayStats& stats)
{
	fillRect(rect, OVERLAY_BAR_BACKGROUND);
	IDWriteTextFormat* format = pBarFormat_.Get();

	// Last results on the left, one fixed-width cell each, oldest first so the newest is on the right
	const float letterWidth = measureText(L"W", format);
	const float cellWidth = measureText(L"W ", format);
	float x = rect.left + CELL_PADDING;
	float resultsRight = x; // the last letter's right edge, without the space after it
	for (auto it = stats.lastResults.rbegin(); it != stats.lastResults.rend(); ++it)
	{
		const bool isWin = *it;
		drawText(isWin ? L"W" : L"L", format, D2D1::RectF(x, rect.top, x + letterWidth, rect.bottom),
			isWin ? OVERLAY_WIN : OVERLAY_LOSS);
		resultsRight = x + letterWidth;
		x += cellWidth;
	}

	// Streak on the right
	const std::wstring streak = L"Streak: " + std::to_wstring(stats.streak);
	const float streakLeft = rect.right - CELL_PADDING - measureText(streak, format);
	drawText(streak, format, D2D1::RectF(streakLeft, rect.top, rect.right - CELL_PADDING, rect.bottom), OVERLAY_TEXT);

	// Winrate centered between them, the value in the win color
	const std::wstring label = L"WINRATE ";
	const std::wstring value = std::to_wstring(stats.winrate) + L"%";
	const float labelWidth = measureText(label, format);
	const float valueWidth = measureText(value, format);
	const float labelLeft = (resultsRight + streakLeft - labelWidth - valueWidth) / 2.0f;
	const float valueLeft = labelLeft + labelWidth;
	drawText(label, format, D2D1::RectF(labelLeft, rect.top, valueLeft, rect.bottom), OVERLAY_TEXT);
	drawText(value, format, D2D1::RectF(valueLeft, rect.top, valueLeft + valueWidth, rect.bottom), OVERLAY_WIN);
}

void MainWindow::drawNameWithRank(const D2D1_RECT_F& cell, const std::wstring& name, const std::optional<int> rank)
{
	IDWriteTextFormat* format = pNamesFormat_.Get();
	const std::wstring suffix = L"(" + formatRank(rank) + L")";
	const float suffixWidth = measureText(suffix, format);

	const float available = cell.right - cell.left - 2 * CELL_PADDING;
	const float nameMaxWidth = (std::max)(available - suffixWidth, 0.0f);
	const float nameWidth = (std::min)(measureText(name, format), nameMaxWidth);

	// Center name and rank as one unit
	const float left = cell.left + (cell.right - cell.left - nameWidth - suffixWidth) / 2.0f;

	ComPtr<IDWriteTextLayout> nameLayout;
	if (nameWidth > 0 && SUCCEEDED(pWriteFactory_->CreateTextLayout(name.c_str(), static_cast<UINT32>(name.size()),
		format, nameMaxWidth, cell.bottom - cell.top, &nameLayout)))
	{
		const DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
		nameLayout->SetTrimming(&trimming, pEllipsis_.Get());
		nameLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		pBrushOverlay_->SetColor(colorRefToColorf(OVERLAY_PALETTE[OVERLAY_TEXT]));
		pRenderTarget_->DrawTextLayout(D2D1::Point2F(left, cell.top), nameLayout.Get(), pBrushOverlay_.Get());
	}

	const float suffixLeft = left + nameWidth;
	drawText(suffix, format, D2D1::RectF(suffixLeft, cell.top, suffixLeft + suffixWidth, cell.bottom), OVERLAY_TEXT);
}

void MainWindow::fillRect(const D2D1_RECT_F& rect, const OverlayColor color)
{
	pBrushOverlay_->SetColor(colorRefToColorf(OVERLAY_PALETTE[color]));
	pRenderTarget_->FillRectangle(rect, pBrushOverlay_.Get());
}

void MainWindow::drawText(const std::wstring& text, IDWriteTextFormat* format, const D2D1_RECT_F& rect, const OverlayColor color)
{
	pBrushOverlay_->SetColor(colorRefToColorf(OVERLAY_PALETTE[color]));
	pRenderTarget_->DrawTextW(text.c_str(), static_cast<UINT32>(text.size()), format, rect, pBrushOverlay_.Get());
}

float MainWindow::measureText(const std::wstring& text, IDWriteTextFormat* format) const
{
	ComPtr<IDWriteTextLayout> layout;
	DWRITE_TEXT_METRICS metrics = {};
	if (FAILED(pWriteFactory_->CreateTextLayout(text.c_str(), static_cast<UINT32>(text.size()), format, 10000.0f, 100.0f, &layout))
		|| FAILED(layout->GetMetrics(&metrics)))
	{
		return 0;
	}
	return metrics.widthIncludingTrailingWhitespace;
}

D2D1_COLOR_F MainWindow::paletteToColorf(const int colorIndex) {
	return colorRefToColorf(PALETTE[colorIndex]);
}

D2D1_COLOR_F MainWindow::colorRefToColorf(const COLORREF color) {
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

			SetTimer(hwnd_, TIMER_ID_FRAME, 16, nullptr);
			// wWinMain sends the first status request; this retries it until a response enables the API
			SetTimer(hwnd_, TIMER_ID_STATUS_RETRY, STATUS_RETRY_MS, nullptr);

			// Settings are loaded before the window is created; the player is fetched once the status is OK
			league_.activePlayerId = appSettings.league.discordId;
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
		case WM_ENTERSIZEMOVE:
			inSizeMove_ = true;
			return 0;
		case WM_EXITSIZEMOVE:
			inSizeMove_ = false;
			setLayout(derivedLayout()); // a layout change that arrived during the drag
			requestRedraw();
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
					if (!statisticsWindow.create(StatsText::WINDOW_TITLE, 500, 200, SIZE_STATISTICS_WIDTH, SIZE_STATISTICS_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, hwnd_, nullptr, nullptr, nullptr)) {
						return 0;
					}

					ShowWindow(statisticsWindow.window(), SW_SHOW);
					refreshPlayerIfNotPolling(); // polling keeps the stats fresh only while the overlay is on
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
			handleStatusResponse(*status);
			break;
		}
		case TIMER_API_PLAYER_RESPONSE:
		{
			const std::unique_ptr<PlayerDetailsDto> playerDetails(reinterpret_cast<PlayerDetailsDto*>(lParam));
			handlePlayerResponse(*playerDetails);
			break;
		}
		case TIMER_API_ERROR:
			handleApiError(static_cast<TimerApiRequest>(lParam), static_cast<DWORD>(wParam));
			break;
		case WM_TIMER:
		{
			switch (wParam)
			{
			case TIMER_ID_FRAME:
				if (timer1.isRunning() || timer2.isRunning()) {
					timer1.updateTime();
					timer2.updateTime();
					requestRedraw();
				}
				break;
			case TIMER_ID_PLAYER_POLL:
				timerApi.getPlayerDetails(league_.activePlayerId, hwnd_);
				break;
			case TIMER_ID_STATUS_RETRY: // repeats until a status response enables the API
				timerApi.getStatus(hwnd_);
				break;
			default:
				break;
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
	case KEY_SCORE_LEFT:
	case KEY_SCORE_RIGHT:
	case KEY_SCORE_RESET:
	{
		if (!league_.overlayStats) return;

		constexpr int maxScore = 99; // keeps the names row from overflowing
		OverlayStats& stats = *league_.overlayStats;
		if (code == KEY_SCORE_LEFT) stats.scorePlayer = (std::min)(stats.scorePlayer + 1, maxScore);
		else if (code == KEY_SCORE_RIGHT) stats.scoreOpponent = (std::min)(stats.scoreOpponent + 1, maxScore);
		else stats.scorePlayer = stats.scoreOpponent = 0;

		requestRedraw();
		return; // Score keys never touch the timers
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

void MainWindow::handleStatusResponse(const StatusDto& status)
{
	league_.slowPollSec = status.slowPollSec;

	if (status.statusEnabled)
	{
		KillTimer(hwnd_, TIMER_ID_STATUS_RETRY);
		if (!league_.activePlayerId.empty()) timerApi.getPlayerDetails(league_.activePlayerId, hwnd_);
		updatePlayerPolling();
	}
	// Otherwise the retry timer is still running and checks again

	statisticsWindow.setStatus(status.statusMessage);
}

void MainWindow::handlePlayerResponse(const PlayerDetailsDto& details)
{
	const bool playerChanged = !league_.pendingPlayerId.empty() && league_.pendingPlayerId != league_.activePlayerId;
	if (!league_.pendingPlayerId.empty())
	{
		league_.activePlayerId = league_.pendingPlayerId;
		league_.pendingPlayerId.clear();
	}

	// The score is the user's, so a poll of the same player keeps it
	OverlayStats stats = toOverlayStats(details);
	if (!playerChanged && league_.overlayStats)
	{
		stats.scorePlayer = league_.overlayStats->scorePlayer;
		stats.scoreOpponent = league_.overlayStats->scoreOpponent;
	}
	league_.overlayStats = std::move(stats);
	statisticsWindow.setModel(toStatsWindowModel(details));
	refreshOverlay();
}

void MainWindow::handleApiError(const TimerApiRequest request, const DWORD code)
{
	league_.pendingPlayerId.clear();

	// The API went down mid-session: stop polling and go back to retrying the status.
	// A failed status needs nothing, its retry is running. Polling survives a 404 or 429 (see isApiOutage),
	// and the overlay keeps its last stats either way.
	if (request == TimerApiRequest::Player && isApiOutage(request, code))
	{
		KillTimer(hwnd_, TIMER_ID_PLAYER_POLL);
		league_.playerPolling = false;
		SetTimer(hwnd_, TIMER_ID_STATUS_RETRY, STATUS_RETRY_MS, nullptr);
	}

	statisticsWindow.setError(request, code);
}

void MainWindow::loadPlayer(const std::wstring& id)
{
	league_.pendingPlayerId = id;
	timerApi.getPlayerDetails(id, hwnd_);
}

void MainWindow::revertPlayer()
{
	league_.pendingPlayerId.clear();

	const std::wstring& savedId = appSettings.league.discordId;
	if (league_.activePlayerId == savedId) return;

	league_.activePlayerId = savedId;
	if (savedId.empty())
	{
		league_.overlayStats.reset();
		statisticsWindow.setModel(std::nullopt);
	}
	else if (timerApiEnabled)
	{
		timerApi.getPlayerDetails(savedId, hwnd_);
	}
	refreshOverlay();
}

void MainWindow::refreshOverlay()
{
	setLayout(derivedLayout());
	updatePlayerPolling();
	requestRedraw();
}

void MainWindow::refreshPlayerIfNotPolling()
{
	if (!league_.playerPolling && !league_.activePlayerId.empty() && timerApiEnabled)
	{
		timerApi.getPlayerDetails(league_.activePlayerId, hwnd_);
	}
}

void MainWindow::updatePlayerPolling()
{
	const bool shouldPoll = timerApiEnabled && appSettings.league.statsOverlayEnabled
		&& !league_.activePlayerId.empty() && league_.slowPollSec > 0;
	if (shouldPoll == league_.playerPolling) return;

	league_.playerPolling = shouldPoll;
	if (shouldPoll)
	{
		SetTimer(hwnd_, TIMER_ID_PLAYER_POLL, static_cast<UINT>(league_.slowPollSec) * 1000, nullptr);
	}
	else
	{
		KillTimer(hwnd_, TIMER_ID_PLAYER_POLL);
	}
}
