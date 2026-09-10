#include "SettingsWindow.h"
#include "ResourceUtils.h"
#include "CommCtrlUtils.h"
#include "SettingsUtils.h"
#include "BaseWindow.h"
#include "Program.h"
#include "ControllerManager.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <exception>

void SettingsWindow::initializeWindow()
{
	// retrieve settings
	tempSettings_ = getSafeSettingsStruct();

	// Set up text
	initializeTextControls();

	// Set up buttons and hotkeys
	initializeButtonControls();

	// Set up bitmaps
	initializeBitmaps();
}

void SettingsWindow::initializeBitmaps()
{
	mouseBitmap_ = loadBitmapResource(IDB_MOUSE);
	controllerBitmap_ = loadBitmapResource(IDB_CONTROLLER);
}

void SettingsWindow::displayBitmaps() const
{
	constexpr int bitmapSize = 25;
	const int mouseX = tileWidth_ * ((cols_ / 2) + 2) - bitmapSize / 2;
	const int mouseY = tileHeight_;
	const int controllerX = tileWidth_ * ((cols_ / 2) + 4) + 10 - bitmapSize / 2;
	const int controllerY = tileHeight_;
	constexpr int originalBitmapSize = 400;
	constexpr COLORREF ignoreColor = RGB(100, 100, 100);

	PAINTSTRUCT ps;
	const HDC hdc = BeginPaint(hwnd_, &ps);

	// Create a compatible device context
	const HDC hMemDc = CreateCompatibleDC(hdc);

	// Select the mouse bitmap into the memory DC
	HGDIOBJ oldBitmap = SelectObject(hMemDc, mouseBitmap_);

	// Scale bitmaps
	TransparentBlt(hdc,
	mouseX, mouseY,			 // Target x, y coordinates
		bitmapSize,				         // Target width
		bitmapSize,						 // Target height
		hMemDc,
		0, 0,			         // Source x, y coordinates
		originalBitmapSize,				 // Source width
		originalBitmapSize,				 // Source height
		ignoreColor);

	// Select the controller bitmap into the memory DC
	oldBitmap = SelectObject(hMemDc, controllerBitmap_);

	// Scale bitmaps
	TransparentBlt(hdc,
		controllerX, controllerY,			// Target x, y coordinates
		bitmapSize,									// Target width
		bitmapSize,								   	// Target height
		hMemDc,
		0, 0,							     // Source x, y coordinates
		originalBitmapSize,							 // Source width
		originalBitmapSize,							 // Source height
		ignoreColor);

	// Cleanup
	SelectObject(hMemDc, oldBitmap);
	DeleteDC(hMemDc);

	EndPaint(hwnd_, &ps);
}

void SettingsWindow::initializeTextControls() const
{
	constexpr int breaklineOffsetY = 25;

	const int headerX = tileWidth_ * (cols_ / 2) - 10;
	const int headerWidth = tileWidth_ + 10;
	constexpr int headerHeight = 20;

	const int titleWidth = tileWidth_ * 6;
	constexpr int titleX = 15;

	// Headers
	const HWND hwndTitleHotkeys = createControl(WC_STATIC, L"Hotkeys", headerX, 5, headerWidth, headerHeight, NULL, SS_CENTER | SS_CENTERIMAGE);
	const HWND hwndTitleOptions = createControl(WC_STATIC, L"Options", headerX, tileHeight_ * 6, headerWidth, headerHeight, NULL, SS_CENTER);
	const HWND hwndTitleColors = createControl(WC_STATIC, L"Colors", headerX, tileHeight_ * 11, headerWidth, headerHeight, NULL, SS_CENTER);

	// Hotkey titles
	const HWND hwndTextStart = createControl(WC_STATIC, L"Start / Stop / Reset", titleX, tileHeight_ * 2, titleWidth, tileHeight_);
	const HWND hwndTextStartNoReset = createControl(WC_STATIC, L"Start / Stop", titleX, tileHeight_ * 3, titleWidth, tileHeight_);
	const HWND hwndTextTimer1 = createControl(WC_STATIC, L"Timer 1", titleX, tileHeight_ * 4, titleWidth, tileHeight_);
	const HWND hwndTextTimer2 = createControl(WC_STATIC, L"Timer 2", titleX, tileHeight_ * 5, titleWidth, tileHeight_);

	// Checkbox titles
	const HWND hwndTextStartOnChange = createControl(WC_STATIC, L"Start Timer On Change", titleX, tileHeight_ * 7, titleWidth, tileHeight_);
	const HWND hwndTextTransparentBackground = createControl(WC_STATIC, L"Transparent Background", titleX, tileHeight_ * 8, titleWidth, tileHeight_);
	const HWND hwndTextCheckboxClickthrough = createControl(WC_STATIC, L"Clickthrough (until restart)", titleX, tileHeight_ * 9, titleWidth, tileHeight_);
	const HWND hwndTextLastSecondsTime = createControl(WC_STATIC, L"Last seconds time", titleX, tileHeight_ * 10, titleWidth, tileHeight_);

	// Break lines
	const HWND hwndBreakLine1 = createControl(WC_STATIC, nullptr, 15, tileHeight_ * 6 + breaklineOffsetY, SIZE_SETTINGS_WIDTH - 40, 5, NULL, SS_ETCHEDHORZ);
	const HWND hwndBreakLine2 = createControl(WC_STATIC, nullptr, 15, tileHeight_ * 11 + breaklineOffsetY, SIZE_SETTINGS_WIDTH - 40, 5, NULL, SS_ETCHEDHORZ);

	// Color options names
	const HWND hwndTextColorTimer = createControl(WC_STATIC, L"Timer", titleX, tileHeight_ * 12, titleWidth, tileHeight_);
	const HWND hwndTextColorSelectedTimer = createControl(WC_STATIC, L"Selected Timer", titleX, tileHeight_ * 13, titleWidth, tileHeight_);
	const HWND hwndTextColorWinCon = createControl(WC_STATIC, L"Last 20 Seconds", titleX, tileHeight_ * 14, titleWidth, tileHeight_);
	const HWND hwndTextColorBackground = createControl(WC_STATIC, L"Background", titleX, tileHeight_ * 15, titleWidth, tileHeight_);

	// Copyright text
	const HWND hwndCopyright = createControl(WC_STATIC, L"\u00A9 Truueh 2026", 10, SIZE_SETTINGS_HEIGHT - 65, 100, 40);

	// Apply fonts
	setControlsFont(hwnd_);
	setTitleFont(hwndTitleColors);
	setTitleFont(hwndTitleHotkeys);
	setCopyrightFont(hwndCopyright);
}

void SettingsWindow::initializeButtonControls()
{
	const int xHotkey = tileWidth_ * ((cols_ / 2) + 1);
	const int xHotkeyCon = tileWidth_ * ((cols_ / 2) + 3) + 10;
	const int xColorButton = tileWidth_ * ((cols_ / 2) + 2);
	const int widthColorButton = tileWidth_ * 3;
	const int heightColorButton = tileHeight_ / 2;
	const int widthHotkey = tileWidth_ * 2;
	const int heightHotkey = tileHeight_ / 2;
	const int widthNumeric = tileWidth_ * 2;
	const int heightNumeric = tileHeight_ / 2;
	const int xNumeric = tileWidth_ * ((cols_ / 2) + 3) + 10;
	constexpr int sizeCheckbox = 15;
	constexpr int xCheckbox = SIZE_SETTINGS_WIDTH - 70;

	HWND hotkeys[8];
	HWND colorButtons[4];

	// Hotkeys
		// Mouse
	hotkeys[0] = createControl(WC_BUTTON, L"", xHotkey, tileHeight_ * 2, widthHotkey, heightHotkey, CID_START, BS_FLAT); // Start key
	hotkeys[3] = createControl(WC_BUTTON, L"", xHotkey, tileHeight_ * 3, widthHotkey, heightHotkey, CID_START_NO_RESET, BS_FLAT); // Start no reset key
	hotkeys[1] = createControl(WC_BUTTON, L"", xHotkey, tileHeight_ * 4, widthHotkey, heightHotkey, CID_TIMER1, BS_FLAT); // Timer 1 key
	hotkeys[2] = createControl(WC_BUTTON, L"", xHotkey, tileHeight_ * 5, widthHotkey, heightHotkey, CID_TIMER2, BS_FLAT); // Timer 2 key

		// Controller
	hotkeys[4] = createControl(WC_BUTTON, L"", xHotkeyCon, tileHeight_ * 2, widthHotkey, heightHotkey, CID_CON_START, BS_FLAT); // Start key
	hotkeys[7] = createControl(WC_BUTTON, L"", xHotkeyCon, tileHeight_ * 3, widthHotkey, heightHotkey, CID_CON_START_NO_RESET, BS_FLAT); // Start no reset key
	hotkeys[5] = createControl(WC_BUTTON, L"", xHotkeyCon, tileHeight_ * 4, widthHotkey, heightHotkey, CID_CON_TIMER1, BS_FLAT); // Timer 1 key
	hotkeys[6] = createControl(WC_BUTTON, L"", xHotkeyCon, tileHeight_ * 5, widthHotkey, heightHotkey, CID_CON_TIMER2, BS_FLAT); // Timer 2 key

	// Checkbox buttons
	const HWND hCbStartOnChange = createControl(WC_BUTTON, L"", xCheckbox, tileHeight_ * 7, sizeCheckbox, sizeCheckbox, CID_STARTONCHANGE_CB, BS_CHECKBOX | BS_AUTOCHECKBOX);
	const HWND hCbTransparentBg = createControl(WC_BUTTON, L"", xCheckbox, tileHeight_ * 8, sizeCheckbox, sizeCheckbox, CID_TRANSPARENT_CB, BS_CHECKBOX | BS_AUTOCHECKBOX);
	const HWND hCbClickthrough = createControl(WC_BUTTON, L"", xCheckbox, tileHeight_ * 9, sizeCheckbox, sizeCheckbox, CID_CLICKTHROUGH_CB, BS_CHECKBOX | BS_AUTOCHECKBOX);
	const HWND hOptLastSecondsTime = createControl(WC_EDIT, L"", xNumeric, tileHeight_ * 10, widthNumeric, heightNumeric, CID_LAST_SECONDS_TIME, ES_NUMBER | ES_CENTER);

	// Color buttons
	colorButtons[0] = createControl(WC_BUTTON, L"", xColorButton, tileHeight_ * 12, widthColorButton, heightColorButton, CID_TIMER_COLOR, BS_OWNERDRAW);
	colorButtons[1] = createControl(WC_BUTTON, L"", xColorButton, tileHeight_ * 13, widthColorButton, heightColorButton, CID_SELECTED_TIMER_COLOR, BS_OWNERDRAW);
	colorButtons[2] = createControl(WC_BUTTON, L"", xColorButton, tileHeight_ * 14, widthColorButton, heightColorButton, CID_LAST_SECONDS_COLOR, BS_OWNERDRAW);
	colorButtons[3] = createControl(WC_BUTTON, L"", xColorButton, tileHeight_ * 15, widthColorButton, heightColorButton, CID_BACKGROUND_COLOR, BS_OWNERDRAW);

	// Initialize exit controls
	HWND hwndOkButton = createControl(WC_BUTTON, L"OK", SIZE_SETTINGS_WIDTH - 160, SIZE_SETTINGS_HEIGHT - 80, 50, 25, CID_OK);
	HWND hwndCancelButton = createControl(WC_BUTTON, L"CANCEL", SIZE_SETTINGS_WIDTH - 100, SIZE_SETTINGS_HEIGHT - 80, 70, 25, CID_CANCEL);

	// Apply font
	setControlsFont(hwnd_);
	
	// Apply currently set hotkeys
	for (const HWND hCtrl : hotkeys) {
		applyHotkeySavedKey(hCtrl);
	}

	// Apply currently set options
	SendMessage(hCbStartOnChange, BM_SETCHECK, appSettings.optionStartOnChange, 0);
	SendMessage(hCbTransparentBg, BM_SETCHECK, appSettings.optionTransparent, 0);
	SendMessage(hCbClickthrough, BM_SETCHECK, appSettings.optionClickThrough, 0);

	// Apply numeric settings
	SendMessage(hOptLastSecondsTime, EM_SETLIMITTEXT, (WPARAM)2, 0); // max 2 digits
	applyInitialControlTextValue(hOptLastSecondsTime);
}

HWND SettingsWindow::createControl(
	const LPCWSTR className, const LPCWSTR controlName, 
	const int x, const int y, const int width, const int height, 
	const int id,const long additionalStyle
) const
{
	const HWND ctrlHwnd = CreateWindowEx(
		0, className, controlName, 
		WS_VISIBLE | WS_CHILDWINDOW | additionalStyle, x,
		y, width, height, hwnd_, (HMENU)id, nullptr, nullptr);

	// Add custom control window proc
	SetWindowSubclass(ctrlHwnd, ctrlWndProc, 1, 0);

	return ctrlHwnd;
}

void SettingsWindow::setCopyrightFont(const HWND hControl)
{
	HFONT hFont = CreateFont(
		14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void SettingsWindow::updateTempSettingsLastSeconds() {
	BOOL success = false;
	int lastSecondsTime = GetDlgItemInt(hwnd_, CID_LAST_SECONDS_TIME, &success, false);
	if (success) {
		tempSettings_.optionLastSecondsTime = lastSecondsTime;
	}
}

void SettingsWindow::handleControlCommand(const LPARAM lParam)
{
	const HWND hwndCtrl = reinterpret_cast<HWND>(lParam); // clicked item handle
	const int controlId = GetDlgCtrlID(hwndCtrl); // retrieve control ID

	// Clicked control
	switch (controlId) {
	// OK
	case CID_OK:
	{
		updateTempSettingsLastSeconds();
		applySettings(tempSettings_);
		SendMessage(GetWindow(hwnd_, GW_OWNER), REFRESH_BRUSHES, 0, 0);
		DestroyWindow(hwnd_);
		break;
	}
		
	// Cancel
	case CID_CANCEL:
		DestroyWindow(hwnd_);
		break;

	// Any hotkey control clicked
	case CID_START:
	case CID_TIMER1:
	case CID_TIMER2:
	case CID_START_NO_RESET:
	case CID_CON_START:
	case CID_CON_TIMER1:
	case CID_CON_TIMER2:
	case CID_CON_START_NO_RESET:
	{
		SetFocus(hwnd_);
		hActiveControl_ = hwndCtrl;
		SetWindowText(hActiveControl_, L"...");
		break;
	}

	// Start on change checkbox
	case CID_STARTONCHANGE_CB:
		tempSettings_.optionStartOnChange = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
		break;

	// Transparent background checkbox
	case CID_TRANSPARENT_CB: 
		tempSettings_.optionTransparent = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
		break;

	// Clickthrough checkbox
	case CID_CLICKTHROUGH_CB:
		tempSettings_.optionClickThrough = (Button_GetCheck(hwndCtrl) == BST_CHECKED);
		break;

	// Any color control clicked
	case CID_TIMER_COLOR:
	case CID_SELECTED_TIMER_COLOR:
	case CID_LAST_SECONDS_COLOR:
	case CID_BACKGROUND_COLOR:
	{
		// Open a Color Picker window
		if (colorPicker.window() == nullptr)
		{
			colorPicker.controlId = controlId; // Notify Color Picker who called it
			colorPicker.pTempSettings = &tempSettings_;
		
			if (!colorPicker.create(L"Color Picker", 850, 300, SIZE_COLORPICKER_WIDTH, SIZE_COLORPICKER_HEIGHT, 0, WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX, hwnd_, nullptr, nullptr)) {
				return;
			}
			ShowWindow(colorPicker.window(), SW_SHOW);
		}
		else
		{
			SetForegroundWindow(colorPicker.window());
		}
	}
	break;
	default:
		break;
	}
}

void SettingsWindow::colorHandles(const LPARAM lParam) const
{
	const LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;
	// In case of invalid color index (to prevent index out of range)
	if (tempSettings_.colors.timerColor > 24) {
		FillRect(pDis->hDC, &pDis->rcItem, hBrushes[0]);
		return;
	}

	// Apply saved color to the color controls in the setting's window
	switch (pDis->CtlID)
	{
	case CID_TIMER_COLOR:
		FillRect(pDis->hDC, &pDis->rcItem, hBrushes[tempSettings_.colors.timerColor]);
		break;
	case CID_SELECTED_TIMER_COLOR:
		FillRect(pDis->hDC, &pDis->rcItem, hBrushes[tempSettings_.colors.selectedTimerColor]);
		break;
	case CID_LAST_SECONDS_COLOR:
		FillRect(pDis->hDC, &pDis->rcItem, hBrushes[tempSettings_.colors.lastSecondsColor]);
		break;
	case CID_BACKGROUND_COLOR:
		FillRect(pDis->hDC, &pDis->rcItem, hBrushes[tempSettings_.colors.backgroundColor]);
		break;
	default:
		break;
	}
}

void SettingsWindow::applyTempConHotkey(const UINT key)
{
	const int controlId = GetDlgCtrlID(hActiveControl_);

	if (controlId == CID_START || controlId == CID_TIMER1 || controlId == CID_TIMER2)
		return;

	switch (controlId)
	{
	case CID_CON_START:
		tempSettings_.conStartKey = key;
		break;
	case CID_CON_TIMER1:
		tempSettings_.conTimer1Key = key;
		break;
	case CID_CON_TIMER2:
		tempSettings_.conTimer2Key = key;
		break;
	case CID_CON_START_NO_RESET:
		tempSettings_.conStartNoResetKey = key;
		break;
	default: 
		break;
	}
	
	SetWindowText(hActiveControl_, controllerMap_[key]);
	hActiveControl_ = nullptr;
}

void SettingsWindow::applyTempHotkey(const UINT key) {
	const int controlId = GetDlgCtrlID(hActiveControl_); // retrieve control ID

	if (controlId == CID_CON_START || controlId == CID_CON_TIMER1 || controlId == CID_CON_TIMER2 || controlId == CID_CON_START_NO_RESET) 
	{
		if (key == VK_ESCAPE)
		{
			applyTempConHotkey(ControllerButtons::Start);

		}
		else if (key == VK_LBUTTON)
		{
			applyTempConHotkey(ControllerButtons::RightTrigger);
		}
		return;
	}

	switch (controlId)
	{
	case CID_START:
		tempSettings_.startKey = key;
		break;
	case CID_TIMER1:
		tempSettings_.timer1Key = key;
		break;
	case CID_TIMER2:
		tempSettings_.timer2Key = key;
		break;
	case CID_START_NO_RESET:
		tempSettings_.startNoResetKey = key;
		break;
	default:
		break;
	}

	SetWindowText(hActiveControl_, keyboardMap_[key]);
	hActiveControl_ = nullptr;
}

void SettingsWindow::applyInitialControlTextValue(const HWND hCtrl) {
	const int controlId = GetDlgCtrlID(hCtrl); // retrieve control ID

	switch (controlId) {
	case CID_LAST_SECONDS_TIME:
	{
		int lastSecondsInt = appSettings.optionLastSecondsTime;
		std::wstring lastSecondsStr = std::to_wstring(lastSecondsInt);
		SetWindowText(hCtrl, lastSecondsStr.c_str());
	}
	default:
		break;
	}
}

void SettingsWindow::applyHotkeySavedKey(const HWND hCtrl) {
	const int controlId = GetDlgCtrlID(hCtrl); // retrieve control ID

	switch (controlId) {
	case CID_START:
		if (keyboardMap_.count(tempSettings_.startKey)) {
			SetWindowText(hCtrl, keyboardMap_[tempSettings_.startKey]);
		}
		break;
	case CID_TIMER1:
		if (keyboardMap_.count(tempSettings_.timer1Key)) {
			SetWindowText(hCtrl, keyboardMap_[tempSettings_.timer1Key]);
		}
		break;
	case CID_TIMER2:
		if (keyboardMap_.count(tempSettings_.timer2Key)) {
			SetWindowText(hCtrl, keyboardMap_[tempSettings_.timer2Key]);
		}
		break;
	case CID_START_NO_RESET:
		if (keyboardMap_.count(tempSettings_.startNoResetKey)) {
			SetWindowText(hCtrl, keyboardMap_[tempSettings_.startNoResetKey]);
		}
		break;
	case CID_CON_START:
		if (controllerMap_.count(tempSettings_.conStartKey)) {
			SetWindowText(hCtrl, controllerMap_[tempSettings_.conStartKey]);
		}
		break;
	case CID_CON_TIMER1:
		if (controllerMap_.count(tempSettings_.conTimer1Key)) {
			SetWindowText(hCtrl, controllerMap_[tempSettings_.conTimer1Key]);
		}
		break;
	case CID_CON_TIMER2:
		if (controllerMap_.count(tempSettings_.conTimer2Key)) {
			SetWindowText(hCtrl, controllerMap_[tempSettings_.conTimer2Key]);
		}
		break;
	case CID_CON_START_NO_RESET:
		if (controllerMap_.count(tempSettings_.conStartNoResetKey)) {
			SetWindowText(hCtrl, controllerMap_[tempSettings_.conStartNoResetKey]);
		}
		break;
	default:
		break;
	}
}

LRESULT SettingsWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
{
	try
	{
		switch (wMsg)
		{
		case WM_CREATE:
		{
			initializeWindow();
			return 0;
		}
		case WM_DESTROY:
		{
			hwnd_ = nullptr;
			DeleteObject(mouseBitmap_);
			DeleteObject(controllerBitmap_);
			mouseBitmap_ = nullptr;
			controllerBitmap_ = nullptr;
			return 0;
		}
		case WM_COMMAND: // Control item clicked
			handleControlCommand(lParam);
			return 0;
		case WM_DRAWITEM: // Color controls
		{
			colorHandles(lParam);
			return TRUE;
		}
		case WM_PAINT:
			displayBitmaps();
			break;
		case WM_XBUTTONDOWN:
			if (hActiveControl_)
			{
				const UINT key = HIWORD(wParam) == 1 ? VK_XBUTTON1 : VK_XBUTTON2;
				applyTempHotkey(key);
			}
			break;
		case WM_MBUTTONDOWN:
			if (hActiveControl_)
			{
				applyTempHotkey(VK_MBUTTON);
			}
			break;
		case WM_RBUTTONDOWN:
			if (hActiveControl_)
			{
				applyTempHotkey(VK_RBUTTON);
			}
			break;
		case WM_LBUTTONDOWN:
			if (hActiveControl_) {
				applyTempHotkey(VK_LBUTTON);
			}
			break;
		case WM_KEYDOWN:
			if (hActiveControl_) {
				const UINT key = (UINT)wParam;
				applyTempHotkey(key);
			}
			break;
		case WM_SYSKEYDOWN: // Specifically ALT key
			if (hActiveControl_ && (UINT)wParam == VK_MENU) {
				const UINT key = (UINT)wParam;
				applyTempHotkey(key);
			}
			break;
		case CONTROLLER_INPUT:
		{
 			if (hActiveControl_) {
				const UINT key = (UINT)wParam;
				applyTempConHotkey(key);
			}
		}
			break;
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

LRESULT CALLBACK SettingsWindow::ctrlWndProc(
	const HWND hwnd, 
	const UINT uMsg, 
	const WPARAM wParam, 
	const LPARAM lParam, 
	const UINT_PTR uIdSubclass,
	const DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		{
			const HWND settingsHwnd = pGlobalTimerWindow->settingsWindow.window();
			const bool isCtrlActive = pGlobalTimerWindow->settingsWindow.hActiveControl_ != nullptr;
			if (settingsHwnd != nullptr && isCtrlActive)
			{
				SendMessage(settingsHwnd, uMsg, wParam, lParam);
				return 0;
			}
		}
		break;
	case WM_LBUTTONDBLCLK: // Redirect double clicks into normal clicks
		SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
		return 0;
	default: 
		break;
	}

	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}