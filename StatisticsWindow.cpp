#include <Windows.h>
#include "Globals.h"
#include "StatisticsWindow.h"
#include "StatisticsWindowStrings.h"
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

	// "Last Matches" columns are sized to their content, with the spare width split into equal gaps
	constexpr int MIN_COLUMN_GAP = 16;

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

	// " (#rank)" or " (?)", after the opponent's name
	std::wstring opponentSuffix(const std::optional<int> rank)
	{
		return L" (" + rankText(rank) + L")";
	}

	// The opponent cell before any cutting: "name (#rank)"
	std::wstring opponentText(const std::wstring& name, const std::optional<int> rank)
	{
		return name + opponentSuffix(rank);
	}

	// Width in pixels of the text in the DC's current font
	int textWidth(const HDC hdc, const std::wstring& text)
	{
		SIZE size = {};
		GetTextExtentPoint32W(hdc, text.c_str(), static_cast<int>(text.size()), &size);
		return size.cx;
	}
}

void StatisticsWindow::setStatus(const std::wstring& message)
{
	statusMessage_ = message;
	setNotice(L"");
	setState(derivedState());
}

void StatisticsWindow::setModel(std::optional<StatsWindowModel> model)
{
	model_ = std::move(model);
	setNotice(L"");
	setState(derivedState());
}

void StatisticsWindow::setError(const TimerApiRequest request, const DWORD code)
{
	if (code == HTTP_STATUS_TOO_MANY_REQUESTS)
	{
		// Not an outage: keep whatever is shown and say so on the notice line.
		// Loading means an active ID with no stats yet, which nothing is fetching anymore.
		setNotice(StatsText::TOO_MANY_REQUESTS_NOTICE);
		const StatsState state = derivedState();
		setState(state == StatsState::Loading ? StatsState::Error : state);
	}
	else if (request == TimerApiRequest::Status)
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

void StatisticsWindow::setNotice(const std::wstring& notice)
{
	if (notice_ == notice) return;
	notice_ = notice;
	if (hwnd_) InvalidateRect(hwnd_, &noticeRect_, TRUE);
}

void StatisticsWindow::initializeWindow()
{
	tempLeague_ = appSettings.league;
	savedLeague_ = appSettings.league;
	applied_ = false;
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
	createControl(WC_STATIC, StatsText::DISCORD_ID_LABEL, MARGIN, MARGIN + 2, idLabelWidth, 20, 0);
	const HWND idEdit = createControl(WC_EDIT, pGlobalTimerWindow->activePlayerId().c_str(), idX, MARGIN, loadX - 10 - idX, 23,
		CID_DISCORD_ID, ES_NUMBER | ES_AUTOHSCROLL | WS_BORDER);
	SendMessage(idEdit, EM_SETLIMITTEXT, 20, 0);
	createControl(WC_BUTTON, StatsText::LOAD_STATS_BUTTON, loadX, MARGIN - 1, LOAD_BUTTON_WIDTH, 26, CID_LOAD_STATS);

	// Overlay options
	createControl(WC_STATIC, StatsText::OVERLAY_LABEL, MARGIN, MARGIN + 40, labelWidth, 20, 0);
	const HWND overlayCheckbox = createControl(WC_BUTTON, L"", checkboxX, MARGIN + 41, CHECKBOX_WIDTH, 17, CID_STATS_OVERLAY_CB, BS_AUTOCHECKBOX);
	createControl(WC_STATIC, StatsText::SHOW_NAMES_LABEL, MARGIN, MARGIN + 70, labelWidth, 20, 0);
	const HWND namesCheckbox = createControl(WC_BUTTON, L"", checkboxX, MARGIN + 71, CHECKBOX_WIDTH, 17, CID_SHOW_NAMES_CB, BS_AUTOCHECKBOX);
	Button_SetCheck(overlayCheckbox, tempLeague_.statsOverlayEnabled ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(namesCheckbox, tempLeague_.showNamesAndScore ? BST_CHECKED : BST_UNCHECKED);

	// Notice line, in the gap between the checkboxes and the stats
	noticeRect_ = { MARGIN, MARGIN + 98, logoX - 12, MARGIN + 120 };

	// Exit buttons: Ok, Cancel, Apply from left to right, the standard Windows order
	const int buttonY = client.bottom - MARGIN - BUTTON_HEIGHT;
	constexpr int buttonStep = BUTTON_WIDTH + 8;
	const int applyX = client.right - MARGIN - BUTTON_WIDTH;
	createControl(WC_BUTTON, StatsText::OK_BUTTON, applyX - 2 * buttonStep, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, CID_OK);
	createControl(WC_BUTTON, StatsText::CANCEL_BUTTON, applyX - buttonStep, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, CID_CANCEL);
	createControl(WC_BUTTON, StatsText::APPLY_BUTTON, applyX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, CID_APPLY);

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
		setNotice(L"");
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
	case CID_APPLY: // in memory only; Ok saves it, Cancel undoes it
		appSettings.league = leagueFromControls();
		applied_ = true;
		pGlobalTimerWindow->updatePlayerPolling();
		break;
	case CID_OK:
	{
		// Apply only the league group onto the current settings, so the settings window's changes stay
		SettingsStruct settings = appSettings;
		settings.league = leagueFromControls();
		applySettings(settings);
		pGlobalTimerWindow->updatePlayerPolling();
		DestroyWindow(hwnd_);
		break;
	}
	case CID_CANCEL:
		if (applied_)
		{
			// Also rewrites the file, in case the settings window saved the applied league meanwhile
			SettingsStruct settings = appSettings;
			settings.league = savedLeague_;
			applySettings(settings);
		}
		pGlobalTimerWindow->revertPlayer(); // back to the saved ID's player
		pGlobalTimerWindow->updatePlayerPolling(); // the overlay flag may have reverted even if the ID didn't
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
	// Ok and Apply take the active ID, which a load in flight may still change
	EnableWindow(GetDlgItem(hwnd_, CID_OK), !loading);
	EnableWindow(GetDlgItem(hwnd_, CID_APPLY), !loading);
}

LeagueSettingsStruct StatisticsWindow::leagueFromControls() const
{
	LeagueSettingsStruct league = tempLeague_;
	league.discordId = pGlobalTimerWindow->activePlayerId(); // the last ID that loaded, not what's typed
	if (league.discordId.empty()) league.statsOverlayEnabled = false;
	return league;
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

	if (!notice_.empty()) drawText(hdc, notice_, noticeRect_, COLOR_LOSS, DT_END_ELLIPSIS);

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
			message = statusMessage_.empty() ? StatsText::DISABLED_MESSAGE : statusMessage_;
			break;
		case StatsState::NeedsId:
			message = StatsText::NEEDS_ID_MESSAGE;
			color = textColor();
			break;
		case StatsState::InvalidId:
			message = StatsText::INVALID_ID_MESSAGE;
			break;
		case StatsState::PlayerNotFound:
			message = StatsText::PLAYER_NOT_FOUND_MESSAGE;
			break;
		case StatsState::Loading:
			message = StatsText::LOADING_MESSAGE;
			color = textColor();
			break;
		default:
			message = StatsText::ERROR_MESSAGE;
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
	drawText(hdc, model.playerName + StatsText::STATS_TITLE_SUFFIX, { left, y, right, y + LINE_HEIGHT }, textColor(), DT_CENTER | DT_END_ELLIPSIS);
	y += LINE_HEIGHT + 6;

	SelectObject(hdc, controlsFont_);
	const std::wstring cells[3][2] = {
		{ StatsText::RANK_LABEL + rankText(model.rank), StatsText::POINTS_LABEL + model.points },
		{ StatsText::TOTAL_GAMES_LABEL + std::to_wstring(model.totalGames), StatsText::WINS_LABEL + std::to_wstring(model.wins) },
		{ StatsText::WINRATE_LABEL + std::to_wstring(model.winrate) + L"%", StatsText::STREAK_LABEL + std::to_wstring(model.streak) },
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
	drawText(hdc, StatsText::LOSSES_LABEL + std::to_wstring(model.losses), { lossesX, y + 2 * LINE_HEIGHT, lossesX + columnWidth, y + 3 * LINE_HEIGHT }, textColor());
	y += 3 * LINE_HEIGHT + 6;

	// Last matches
	drawSeparator(hdc, left, right, y);
	y += SECTION_GAP;
	SelectObject(hdc, titleFont_);
	drawText(hdc, StatsText::MATCHES_TITLE, { left, y, right, y + LINE_HEIGHT }, textColor(), DT_CENTER);
	y += LINE_HEIGHT + 4;
	SelectObject(hdc, controlsFont_);

	const auto rowRect = [&](const int x1, const int x2) { return RECT{ x1, y, x2, y + MATCH_ROW_HEIGHT }; };

	if (model.matches.empty())
	{
		drawText(hdc, StatsText::NO_MATCHES_MESSAGE, rowRect(left, right), textColor());
		return;
	}

	// Each column is as wide as its widest header or cell
	enum Column { Date, Result, Score, Opponent, Points, ColumnCount };
	int widths[ColumnCount] = {
		textWidth(hdc, StatsText::DATE_HEADER),
		(std::max)({ textWidth(hdc, StatsText::RESULT_HEADER), textWidth(hdc, StatsText::WIN_RESULT), textWidth(hdc, StatsText::LOSS_RESULT) }),
		textWidth(hdc, StatsText::SCORE_HEADER),
		textWidth(hdc, StatsText::OPPONENT_HEADER),
		textWidth(hdc, StatsText::POINTS_GAINED_HEADER),
	};
	for (const MatchRow& match : model.matches)
	{
		widths[Date] = (std::max)(widths[Date], textWidth(hdc, match.date));
		widths[Score] = (std::max)(widths[Score], textWidth(hdc, match.score));
		widths[Opponent] = (std::max)(widths[Opponent], textWidth(hdc, opponentText(match.opponentName, match.opponentRank)));
		widths[Points] = (std::max)(widths[Points], textWidth(hdc, match.pointsGained));
	}

	// The spare width becomes equal gaps. If there's too little, only the opponent column shrinks
	constexpr int gapCount = ColumnCount - 1;
	const int available = right - left;
	int content = 0;
	for (const int width : widths) content += width;
	int gap = (available - content) / gapCount;
	if (gap < MIN_COLUMN_GAP)
	{
		gap = MIN_COLUMN_GAP;
		widths[Opponent] = available - (content - widths[Opponent]) - gapCount * MIN_COLUMN_GAP;
	}

	// Left edges; the last column ends at the right edge, which absorbs the rounding
	int xs[ColumnCount] = {};
	xs[Date] = left;
	for (int column = 1; column < Points; ++column) xs[column] = xs[column - 1] + widths[column - 1] + gap;
	xs[Points] = right - widths[Points];
	const auto cell = [&](const Column column) { return rowRect(xs[column], xs[column] + widths[column]); };

	// Everything is left-aligned except the score, which is centered
	drawText(hdc, StatsText::DATE_HEADER, cell(Date), textColor());
	drawText(hdc, StatsText::RESULT_HEADER, cell(Result), textColor());
	drawText(hdc, StatsText::SCORE_HEADER, cell(Score), textColor(), DT_CENTER);
	drawText(hdc, StatsText::OPPONENT_HEADER, cell(Opponent), textColor());
	drawText(hdc, StatsText::POINTS_GAINED_HEADER, cell(Points), textColor());
	y += MATCH_ROW_HEIGHT;

	for (const MatchRow& match : model.matches)
	{
		drawText(hdc, match.date, cell(Date), textColor());
		drawText(hdc, match.isWin ? StatsText::WIN_RESULT : StatsText::LOSS_RESULT, cell(Result), match.isWin ? COLOR_GAIN : COLOR_LOSS);
		drawText(hdc, match.score, cell(Score), textColor(), DT_CENTER);
		SetTextColor(hdc, textColor());
		drawOpponent(hdc, cell(Opponent), match.opponentName, match.opponentRank);
		drawText(hdc, match.pointsGained, cell(Points), match.pointsPositive ? COLOR_GAIN : COLOR_LOSS);
		y += MATCH_ROW_HEIGHT;
	}
}

void StatisticsWindow::drawOpponent(const HDC hdc, RECT rect, const std::wstring& name, const std::optional<int> rank)
{
	const int available = rect.right - rect.left;
	std::wstring text = opponentText(name, rank);
	if (textWidth(hdc, text) > available)
	{
		// Fit as much of the name as leaves room for the ellipsis and the rank
		const std::wstring suffix = opponentSuffix(rank);
		const int nameRoom = (std::max)(0, available - textWidth(hdc, suffix) - textWidth(hdc, L"\u2026"));
		int fit = 0;
		SIZE fitSize;
		GetTextExtentExPointW(hdc, name.c_str(), static_cast<int>(name.size()), nameRoom, &fit, nullptr, &fitSize);
		text = name.substr(0, fit) + L"\u2026" + suffix;
	}
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
