#include <Windows.h>
#include "Globals.h"
#include "StatisticsWindow.h"
#include "CommCtrlUtils.h"
#include "ResourceUtils.h"
#include "SettingsUtils.h"
#include "Program.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <algorithm>
#include <exception>
#include <winhttp.h>

namespace
{
	// Layout, in client pixels
	constexpr int MARGIN = 16;
	constexpr int LOGO_SIZE = 120;		 // the bitmap's own size, so it's drawn unscaled
	constexpr int LOAD_BUTTON_WIDTH = 90;
	constexpr int CHECKBOX_WIDTH = 13;	 // same as the glyph, so its right edge lines up with Load Stats
	constexpr int BUTTON_WIDTH = 75;
	constexpr int BUTTON_HEIGHT = 28;
	constexpr int LINE_HEIGHT = 22;		 // a line of painted text
	constexpr int MATCH_ROW_HEIGHT = 21;
	constexpr int SECTION_GAP = 8;

	// "Last Matches" column widths; the opponent takes the rest
	constexpr int DATE_WIDTH = 135;
	constexpr int RESULT_WIDTH = 55;
	constexpr int SCORE_WIDTH = 55;
	constexpr int POINTS_WIDTH = 105;

	constexpr COLORREF COLOR_GAIN = RGB(0, 128, 0);
	constexpr COLORREF COLOR_LOSS = RGB(200, 30, 30);

	COLORREF textColor() { return GetSysColor(COLOR_WINDOWTEXT); }

	void drawText(const HDC hdc, const std::wstring& text, RECT rect, const COLORREF color, const UINT format = 0)
	{
		SetTextColor(hdc, color);
		DrawTextW(hdc, text.c_str(), static_cast<int>(text.size()), &rect, format | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
	}

	void drawSeparator(const HDC hdc, const int left, const int right, const int y)
	{
		RECT line = { left, y, right, y + 2 };
		DrawEdge(hdc, &line, EDGE_ETCHED, BF_TOP);
	}

	std::wstring rankText(const std::optional<int> rank)
	{
		return rank ? L"#" + std::to_wstring(*rank) : L"?";
	}
}

void StatisticsWindow::setStatus(const std::wstring& message)
{
	statusMessage_ = message;
	setState(derivedState());
}

void StatisticsWindow::setModel(std::optional<StatsWindowModel> model)
{
	model_ = std::move(model);
	setState(derivedState());
}

void StatisticsWindow::setError(const TimerApiRequest request, const DWORD code)
{
	if (request == TimerApiRequest::Status)
	{
		statusMessage_.clear(); // the request failed, so there's no message from the API
		setState(StatsState::Disabled);
	}
	else
	{
		setState(code == HTTP_STATUS_NOT_FOUND ? StatsState::PlayerNotFound : StatsState::Error);
	}
}

StatsState StatisticsWindow::derivedState() const
{
	if (!timerApiEnabled) return StatsState::Disabled;
	if (model_) return StatsState::Loaded;
	if (pGlobalTimerWindow && !pGlobalTimerWindow->activePlayerId().empty()) return StatsState::Loading;
	return StatsState::NeedsId;
}

void StatisticsWindow::setState(const StatsState state)
{
	state_ = state;
	if (!hwnd_) return; // shown when the window opens

	updateControls();
	InvalidateRect(hwnd_, &contentRect_, TRUE);
}

void StatisticsWindow::initializeWindow()
{
	tempLeague_ = appSettings.league;
	controlsFont_ = createControlsFont();
	titleFont_ = createTitleFont();
	logo_ = loadBitmapResource(IDB_LEAGUE_LOGO);

	RECT client;
	GetClientRect(hwnd_, &client);
	const int logoX = client.right - MARGIN - LOGO_SIZE;
	const int loadX = logoX - 12 - LOAD_BUTTON_WIDTH;
	const int checkboxX = loadX + LOAD_BUTTON_WIDTH - CHECKBOX_WIDTH;
	const int labelWidth = checkboxX - MARGIN - 8;
	constexpr int idLabelWidth = 130;
	constexpr int idX = MARGIN + idLabelWidth + 5;

	// Discord ID row
	createControl(WC_STATIC, L"Your Discord ID", MARGIN, MARGIN + 2, idLabelWidth, 20, 0);
	const HWND idEdit = createControl(WC_EDIT, pGlobalTimerWindow->activePlayerId().c_str(), idX, MARGIN, loadX - 10 - idX, 23,
		CID_DISCORD_ID, ES_NUMBER | ES_AUTOHSCROLL | WS_BORDER);
	SendMessage(idEdit, EM_SETLIMITTEXT, 20, 0);
	createControl(WC_BUTTON, L"Load Stats", loadX, MARGIN - 1, LOAD_BUTTON_WIDTH, 26, CID_LOAD_STATS);

	// Overlay options
	createControl(WC_STATIC, L"Enable overlay stats", MARGIN, MARGIN + 40, labelWidth, 20, 0);
	const HWND overlayCheckbox = createControl(WC_BUTTON, L"", checkboxX, MARGIN + 41, CHECKBOX_WIDTH, 17, CID_STATS_OVERLAY_CB, BS_AUTOCHECKBOX);
	createControl(WC_STATIC, L"Show names and score", MARGIN, MARGIN + 70, labelWidth, 20, 0);
	const HWND namesCheckbox = createControl(WC_BUTTON, L"", checkboxX, MARGIN + 71, CHECKBOX_WIDTH, 17, CID_SHOW_NAMES_CB, BS_AUTOCHECKBOX);
	Button_SetCheck(overlayCheckbox, tempLeague_.statsOverlayEnabled ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(namesCheckbox, tempLeague_.showNamesAndScore ? BST_CHECKED : BST_UNCHECKED);

	// Exit buttons
	const int buttonY = client.bottom - MARGIN - BUTTON_HEIGHT;
	createControl(WC_BUTTON, L"Ok", client.right - MARGIN - BUTTON_WIDTH, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, CID_OK);
	createControl(WC_BUTTON, L"Cancel", client.right - MARGIN - 2 * BUTTON_WIDTH - 8, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, CID_CANCEL);

	contentRect_ = { 0, MARGIN + LOGO_SIZE + SECTION_GAP, client.right, buttonY - 4 };
	setState(derivedState());
}

HWND StatisticsWindow::createControl(
	const LPCWSTR className, const LPCWSTR text,
	const int x, const int y, const int width, const int height,
	const int id, const long additionalStyle) const
{
	const HWND ctrlHwnd = CreateWindowEx(0, className, text, WS_VISIBLE | WS_CHILDWINDOW | additionalStyle,
		x, y, width, height, hwnd_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
	SendMessage(ctrlHwnd, WM_SETFONT, reinterpret_cast<WPARAM>(controlsFont_), TRUE);
	SetWindowSubclass(ctrlHwnd, ctrlWndProc, 1, 0);
	return ctrlHwnd;
}

void StatisticsWindow::handleControlCommand(const int controlId)
{
	switch (controlId)
	{
	case CID_LOAD_STATS:
	{
		wchar_t buffer[32] = {};
		GetDlgItemTextW(hwnd_, CID_DISCORD_ID, buffer, ARRAYSIZE(buffer));
		const std::wstring id = buffer;
		if (!isValidDiscordId(id))
		{
			setState(StatsState::InvalidId);
			break;
		}
		setState(StatsState::Loading);
		pGlobalTimerWindow->loadPlayer(id);
		break;
	}
	case CID_STATS_OVERLAY_CB:
		tempLeague_.statsOverlayEnabled = IsDlgButtonChecked(hwnd_, CID_STATS_OVERLAY_CB) == BST_CHECKED;
		updateControls(); // "Show names and score" depends on it
		break;
	case CID_SHOW_NAMES_CB:
		tempLeague_.showNamesAndScore = IsDlgButtonChecked(hwnd_, CID_SHOW_NAMES_CB) == BST_CHECKED;
		break;
	case CID_OK:
	{
		// Apply only the league group onto the current settings, so the settings window's changes stay
		SettingsStruct settings = appSettings;
		settings.league = tempLeague_;
		settings.league.discordId = pGlobalTimerWindow->activePlayerId(); // the last ID that loaded, not what's typed
		if (settings.league.discordId.empty()) settings.league.statsOverlayEnabled = false;
		applySettings(settings);
		pGlobalTimerWindow->updatePlayerPolling();
		DestroyWindow(hwnd_);
		break;
	}
	case CID_CANCEL:
		pGlobalTimerWindow->revertPlayer();
		DestroyWindow(hwnd_);
		break;
	default:
		break;
	}
}

void StatisticsWindow::updateControls() const
{
	const bool apiUp = state_ != StatsState::Disabled;
	const bool loaded = state_ == StatsState::Loaded;
	const bool loading = state_ == StatsState::Loading;

	EnableWindow(GetDlgItem(hwnd_, CID_DISCORD_ID), apiUp);
	EnableWindow(GetDlgItem(hwnd_, CID_LOAD_STATS), apiUp && !loading);
	EnableWindow(GetDlgItem(hwnd_, CID_STATS_OVERLAY_CB), loaded);
	EnableWindow(GetDlgItem(hwnd_, CID_SHOW_NAMES_CB), loaded && tempLeague_.statsOverlayEnabled);
	EnableWindow(GetDlgItem(hwnd_, CID_OK), !loading); // Ok saves the active ID, which a load in flight may still change
}

void StatisticsWindow::paint() const
{
	PAINTSTRUCT ps;
	const HDC hdc = BeginPaint(hwnd_, &ps);

	if (logo_)
	{
		const HDC memDc = CreateCompatibleDC(hdc);
		const HGDIOBJ oldBitmap = SelectObject(memDc, logo_);
		BitBlt(hdc, contentRect_.right - MARGIN - LOGO_SIZE, MARGIN, LOGO_SIZE, LOGO_SIZE, memDc, 0, 0, SRCCOPY);
		SelectObject(memDc, oldBitmap);
		DeleteDC(memDc);
	}

	SetBkMode(hdc, TRANSPARENT);
	const HGDIOBJ oldFont = SelectObject(hdc, controlsFont_);

	if (state_ == StatsState::Loaded && model_)
	{
		paintStats(hdc);
	}
	else
	{
		std::wstring message;
		COLORREF color = COLOR_LOSS;
		switch (state_)
		{
		case StatsState::Disabled:
			message = statusMessage_.empty() ? L"The stats feature is currently disabled" : statusMessage_;
			break;
		case StatsState::NeedsId:
			message = L"Enter your discord id to enable this feature";
			color = textColor();
			break;
		case StatsState::InvalidId:
			message = L"Enter a valid discord id (17-20 digits)";
			break;
		case StatsState::PlayerNotFound:
			message = L"No player was found with this discord id";
			break;
		case StatsState::Loading:
			message = L"Loading stats...";
			color = textColor();
			break;
		default:
			message = L"Couldn't load the stats, try again later";
			break;
		}

		RECT rect = { MARGIN, contentRect_.top, contentRect_.right - MARGIN, contentRect_.bottom };
		SetTextColor(hdc, color);
		DrawTextW(hdc, message.c_str(), static_cast<int>(message.size()), &rect, DT_WORDBREAK | DT_NOPREFIX);
	}

	SelectObject(hdc, oldFont);
	EndPaint(hwnd_, &ps);
}

void StatisticsWindow::paintStats(const HDC hdc) const
{
	const StatsWindowModel& model = *model_;
	const int left = MARGIN;
	const int right = contentRect_.right - MARGIN;
	int y = contentRect_.top;

	// Player stats
	drawSeparator(hdc, left, right, y);
	y += SECTION_GAP;
	SelectObject(hdc, titleFont_);
	drawText(hdc, model.playerName + L"'s Stats", { left, y, right, y + LINE_HEIGHT }, textColor(), DT_CENTER | DT_END_ELLIPSIS);
	y += LINE_HEIGHT + 6;

	SelectObject(hdc, controlsFont_);
	const std::wstring cells[3][2] = {
		{ L"Rank: " + rankText(model.rank), L"Points: " + model.points },
		{ L"Total games: " + std::to_wstring(model.totalGames), L"Wins: " + std::to_wstring(model.wins) },
		{ L"Winrate: " + std::to_wstring(model.winrate) + L"%", L"Streak: " + std::to_wstring(model.streak) },
	};
	const int columnWidth = (right - left) / 3;
	for (int column = 0; column < 3; ++column)
	{
		const int x = left + column * columnWidth;
		for (int row = 0; row < 2; ++row)
		{
			drawText(hdc, cells[column][row], { x, y + row * LINE_HEIGHT, x + columnWidth, y + (row + 1) * LINE_HEIGHT }, textColor());
		}
	}
	const int lossesX = left + columnWidth;
	drawText(hdc, L"Losses: " + std::to_wstring(model.losses), { lossesX, y + 2 * LINE_HEIGHT, lossesX + columnWidth, y + 3 * LINE_HEIGHT }, textColor());
	y += 3 * LINE_HEIGHT + 6;

	// Last matches
	drawSeparator(hdc, left, right, y);
	y += SECTION_GAP;
	SelectObject(hdc, titleFont_);
	drawText(hdc, L"Last Matches", { left, y, right, y + LINE_HEIGHT }, textColor(), DT_CENTER);
	y += LINE_HEIGHT + 4;
	SelectObject(hdc, controlsFont_);

	const int resultX = left + DATE_WIDTH;
	const int scoreX = resultX + RESULT_WIDTH;
	const int opponentX = scoreX + SCORE_WIDTH;
	const int pointsX = right - POINTS_WIDTH;
	const auto rowRect = [&](const int x1, const int x2) { return RECT{ x1, y, x2, y + MATCH_ROW_HEIGHT }; };

	if (model.matches.empty())
	{
		drawText(hdc, L"No matches played yet", rowRect(left, right), textColor());
		return;
	}

	drawText(hdc, L"Date", rowRect(left, resultX), textColor());
	drawText(hdc, L"Result", rowRect(resultX, scoreX), textColor());
	drawText(hdc, L"Score", rowRect(scoreX, opponentX), textColor());
	drawText(hdc, L"Opponent", rowRect(opponentX, pointsX), textColor());
	drawText(hdc, L"Points Gained", rowRect(pointsX, right), textColor());
	y += MATCH_ROW_HEIGHT;

	for (const MatchRow& match : model.matches)
	{
		drawText(hdc, match.date, rowRect(left, resultX), textColor());
		drawText(hdc, match.isWin ? L"Win" : L"Loss", rowRect(resultX, scoreX), match.isWin ? COLOR_GAIN : COLOR_LOSS);
		drawText(hdc, match.score, rowRect(scoreX, opponentX), textColor());
		SetTextColor(hdc, textColor());
		drawOpponent(hdc, rowRect(opponentX, pointsX - 8), match.opponentName, match.opponentRank);
		drawText(hdc, match.pointsGained, rowRect(pointsX, right), match.pointsPositive ? COLOR_GAIN : COLOR_LOSS);
		y += MATCH_ROW_HEIGHT;
	}
}

void StatisticsWindow::drawOpponent(const HDC hdc, RECT rect, const std::wstring& name, const std::optional<int> rank)
{
	const std::wstring suffix = L" (" + rankText(rank) + L")";
	const int available = rect.right - rect.left;

	SIZE suffixSize, nameSize;
	GetTextExtentPoint32W(hdc, suffix.c_str(), static_cast<int>(suffix.size()), &suffixSize);
	GetTextExtentPoint32W(hdc, name.c_str(), static_cast<int>(name.size()), &nameSize);

	std::wstring text = name;
	if (nameSize.cx + suffixSize.cx > available)
	{
		// Fit as much of the name as leaves room for the ellipsis and the rank
		SIZE ellipsisSize;
		GetTextExtentPoint32W(hdc, L"\u2026", 1, &ellipsisSize);
		const int nameRoom = (std::max)(0, static_cast<int>(available - suffixSize.cx - ellipsisSize.cx));
		int fit = 0;
		GetTextExtentExPointW(hdc, name.c_str(), static_cast<int>(name.size()), nameRoom, &fit, nullptr, &nameSize);
		text = name.substr(0, fit) + L"\u2026";
	}
	text += suffix;
	DrawTextW(hdc, text.c_str(), static_cast<int>(text.size()), &rect, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
}

LRESULT CALLBACK StatisticsWindow::ctrlWndProc(
	const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam,
	const UINT_PTR uIdSubclass, const DWORD_PTR dwRefData)
{
	if (wParam == VK_ESCAPE)
	{
		// Posted: Cancel destroys this control, so it can't happen inside the control's own message
		if (uMsg == WM_KEYDOWN) PostMessage(GetParent(hwnd), WM_COMMAND, CID_CANCEL, 0);
		if (uMsg == WM_KEYDOWN || uMsg == WM_CHAR) return 0; // WM_CHAR would make the edit control beep
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT StatisticsWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
{
	try
	{
		switch (wMsg)
		{
		case WM_CREATE:
			initializeWindow();
			return 0;
		case WM_COMMAND: // LOWORD is the control ID, also for the Cancel that ctrlWndProc posts
			handleControlCommand(LOWORD(wParam));
			return 0;
		case WM_PAINT:
			paint();
			return 0;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) handleControlCommand(CID_CANCEL);
			return 0;
		case WM_CLOSE: // e.g. Alt+F4: same as Cancel, so an unsaved player is reverted
			handleControlCommand(CID_CANCEL);
			return 0;
		case WM_DESTROY:
			hwnd_ = nullptr;
			DeleteObject(logo_);
			logo_ = nullptr;
			return 0;
		case WM_NCDESTROY: // sent after the child controls are destroyed
			DeleteObject(controlsFont_);
			DeleteObject(titleFont_);
			controlsFont_ = nullptr;
			titleFont_ = nullptr;
			return 0;
		default:
			break;
		}
	}
	catch (const std::exception &e)
	{
		exitApp();
	}
	return DefWindowProc(window(), wMsg, wParam, lParam);
}
