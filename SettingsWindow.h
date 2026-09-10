#pragma once
#include "ColorPickerWindow.h"
#include "Globals.h"
#include <map>

// The class responsible for the settings window
class SettingsWindow : public BaseWindow<SettingsWindow>
{
private:
	// Fields
	SettingsStruct tempSettings_ = {};
	HBITMAP mouseBitmap_ = nullptr;
	HBITMAP controllerBitmap_ = nullptr;
	HWND hActiveControl_ = nullptr;

	byte rows_ = 18;
	byte cols_ = 11;

	int tileHeight_ = SIZE_SETTINGS_HEIGHT / rows_;
	int tileWidth_ = SIZE_SETTINGS_WIDTH / cols_;

	/**
	@brief Calls required methods to initialize the settings window.
	*/
	void initializeWindow();

	/**
	@brief Initializes the bitmaps for the settings window.
	*/
	void initializeBitmaps();

	/**
	@brief Displays the bitmaps of the settings window. (controller & mouse images)
	*/
	void displayBitmaps() const;

	/**
	@brief Initializes all the text controls in the settings window.
	*/
	void initializeTextControls() const;
	
	/**
	@brief Initializes all the interactable controls in the settings window.
	*/
	void initializeButtonControls();

	/**
	@brief Creates a control and returns it.

	@param className The LPCWSTR representing the control's class name.

	@param controlName LPCWSTR representing the control's name.

	@param x Integer value representing the relative x position of the control on the window.

	@param y Integer value representing the relative y position of the control on the window.

	@param width Integer value to set the width of the control

	@param height Integer value to set the height of the control

	@param id Set an identifier for the control. Can be NULL.

	@param ADDITIONAL_STYLE Applies additional styles.

	@return The HWND control that was created.
	*/
	HWND createControl(
		LPCWSTR className, LPCWSTR controlName, 
		int x, int y, int width, int height, 
		int id = NULL, long additionalStyle = 0
	) const;

	/**
	@brief Sets the "copyright" font to a given control.

	@param hControl The handle to the control to set the font of.
	*/
	static void setCopyrightFont(HWND hControl);

	/**
	@brief Method that handles interaction with controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void handleControlCommand(LPARAM lParam);

	/**
	@brief Saves the key to the temp SettingsStruct (as the currently selected control).

	@param key The key that the user hit.
	*/
	void applyTempConHotkey(const UINT key);

	/**
	@brief Saves the key to the temp SettingsStruct (as the currently selected control).

	@param key The key that the user hit.
	*/
	void applyTempHotkey(UINT key);

	/**
	@brief Set the text of the given control to it's saved value in the tempSettings_ struct.

	@param hCtrl The HWND instance to set the text of.
	*/
	void applyHotkeySavedKey(HWND hCtrl);

	/**
	@brief Set the initial value of a given text control with it's value from the settings

	@param hCtrl The HWND instance to set the text of
	*/
	void applyInitialControlTextValue(HWND hCtrl);

	/**
	@brief Apply the new LastSecondsTime field to the tempSettings_
	*/
	void updateTempSettingsLastSeconds();

	/**
	@brief WindowProcedure for CommonControls controls.

	@param uMsg The WindowMessage

	@param wParam WPARAM argument

	@param lParam LPARAM argument

	@uIdSubclass A pointer to uniquely identify this subclass callback

	@param dwRefData A pointer to a DWORD which will return the reference data
	*/
	static LRESULT CALLBACK ctrlWndProc(
		HWND hwnd, 
		const UINT uMsg, 
		const WPARAM wParam, 
		const LPARAM lParam,
		const UINT_PTR uIdSubclass,
		const DWORD_PTR dwRefData);

	// Map VirtualKey codes to wstrings representing them.
	std::map<UINT, LPCWSTR> keyboardMap_ = {
		{VK_LBUTTON, L"LButton"},
		{VK_RBUTTON, L"RButton"},
		{VK_MBUTTON, L"MWheel"},
		{VK_XBUTTON1, L"MBack"},
		{VK_XBUTTON2, L"MFront"},
		{VK_MENU, L"ALT"},
		{VK_CONTROL, L"CTRL"},
		{VK_SHIFT, L"Shift"},
		{VK_CAPITAL, L"CAPS"},
		{VK_TAB, L"TAB"},
		{VK_ESCAPE, L"ESC"},
		{VK_OEM_3, L"`"},
		{VK_LEFT, L"Left"},
		{VK_RIGHT, L"Right"},
		{VK_UP, L"Up"},
		{VK_DOWN, L"Down"},
		{VK_RETURN, L"Enter"},
		{VK_BACK, L"Backspace"},
		{VK_SPACE, L"Spacebar"},
		{VK_CLEAR, L"Clear"},
		{VK_PAUSE, L"Pause"},
		{VK_PRIOR, L"Page Up"},
		{VK_NEXT, L"Page Down"},
		{VK_END, L"End"},
		{VK_HOME, L"Home"},
		{VK_SELECT, L"Select"},
		{VK_PRINT, L"Print"},
		{VK_EXECUTE, L"Execute"},
		{VK_SNAPSHOT, L"Snapshot"},
		{VK_INSERT, L"Insert"},
		{VK_DELETE, L"Delete"},
		{VK_HELP, L"Help"},
		{0x30, L"0"},
		{0x31, L"1"},
		{0x32, L"2"},
		{0x33, L"3"},
		{0x34, L"4"},
		{0x35, L"5"},
		{0x36, L"6"},
		{0x37, L"7"},
		{0x38, L"8"},
		{0x39, L"9"},
		{0x41, L"A"},
		{0x42, L"B"},
		{0x43, L"C"},
		{0x44, L"D"},
		{0x45, L"E"},
		{0x46, L"F"},
		{0x47, L"G"},
		{0x48, L"H"},
		{0x49, L"I"},
		{0x4A, L"J"},
		{0x4B, L"K"},
		{0x4C, L"L"},
		{0x4D, L"M"},
		{0x4E, L"N"},
		{0x4F, L"O"},
		{0x50, L"P"},
		{0x51, L"Q"},
		{0x52, L"R"},
		{0x53, L"S"},
		{0x54, L"T"},
		{0x55, L"U"},
		{0x56, L"V"},
		{0x57, L"W"},
		{0x58, L"X"},
		{0x59, L"Y"},
		{0x5A, L"Z"},
		{VK_LWIN, L"Left Win"},
		{VK_RWIN, L"Right Win"},
		{VK_APPS, L"Apps"},
		{VK_SLEEP, L"Sleep"},
		{VK_NUMPAD0, L"Num 0"},
		{VK_NUMPAD1, L"Num 1"},
		{VK_NUMPAD2, L"Num 2"},
		{VK_NUMPAD3, L"Num 3"},
		{VK_NUMPAD4, L"Num 4"},
		{VK_NUMPAD5, L"Num 5"},
		{VK_NUMPAD6, L"Num 6"},
		{VK_NUMPAD7, L"Num 7"},
		{VK_NUMPAD8, L"Num 8"},
		{VK_NUMPAD9, L"Num 9"},
		{VK_MULTIPLY, L"Mul"},
		{VK_ADD, L"Add"},
		{VK_SEPARATOR, L"Seperator"},
		{VK_SUBTRACT, L"Sub"},
		{VK_DECIMAL, L"Decimal"},
		{VK_DIVIDE, L"Divide"},
		{VK_F1, L"F1"},
		{VK_F2, L"F2"},
		{VK_F3, L"F3"},
		{VK_F4, L"F4"},
		{VK_F5, L"F5"},
		{VK_F6, L"F6"},
		{VK_F7, L"F7"},
		{VK_F8, L"F8"},
		{VK_F9, L"F9"},
		{VK_F10, L"F10"},
		{VK_F11, L"F11"},
		{VK_F12, L"F12"},
		{VK_F13, L"F13"},
		{VK_F14, L"F14"},
		{VK_F15, L"F15"},
		{VK_F16, L"F16"},
		{VK_F17, L"F17"},
		{VK_F18, L"F18"},
		{VK_F19, L"F19"},
		{VK_F20, L"F20"},
		{VK_F21, L"F21"},
		{VK_F22, L"F22"},
		{VK_F23, L"F23"},
		{VK_F24, L"F24"},
		{VK_NUMLOCK, L"Numlock"},
		{VK_SCROLL, L"Scroll"},
		{VK_BROWSER_BACK, L"Back"},
		{VK_BROWSER_FORWARD, L"Forward"},
		{VK_BROWSER_REFRESH, L"Refresh"},
		{VK_BROWSER_STOP, L"Stop"},
		{VK_BROWSER_SEARCH, L"Search"},
		{VK_BROWSER_FAVORITES, L"Favorites"},
		{VK_BROWSER_HOME, L"Home"},
		{VK_VOLUME_MUTE, L"Mute"},
		{VK_VOLUME_DOWN, L"Vol Down"},
		{VK_VOLUME_UP, L"Vol Up"},
		{VK_MEDIA_NEXT_TRACK, L"Next"},
		{VK_MEDIA_PREV_TRACK, L"Prev"},
		{VK_MEDIA_STOP, L"Stop"},
		{VK_MEDIA_PLAY_PAUSE, L"Play"},
		{VK_LAUNCH_MAIL, L"Mail"},
		{VK_LAUNCH_MEDIA_SELECT, L"Select"},
		{VK_LAUNCH_APP1, L"App 1"},
		{VK_LAUNCH_APP2, L"App 2"},
		{VK_OEM_1, L";"},
		{VK_OEM_PLUS, L"="},
		{VK_OEM_COMMA, L","},
		{VK_OEM_PERIOD, L"."},
		{VK_OEM_MINUS, L"-"},
		{VK_OEM_2, L"/"},
		{VK_OEM_3, L"`"},
		{VK_OEM_4, L"["},
		{VK_OEM_5, L"\\"},
		{VK_OEM_6, L"]"},
		{VK_OEM_7, L"'"},
		{VK_OEM_8, L"?"},
		{VK_ATTN, L"Attn"},
		{VK_CRSEL, L"Crsel"},
		{VK_EXSEL, L"Exsel"},
		{VK_EREOF, L"Ereof"},
		{VK_PLAY, L"Play"},
		{VK_ZOOM, L"Zoom"},
		{VK_PA1, L"PA1"},
		{VK_OEM_CLEAR, L"Clear"},
	};

	std::map<UINT, LPCWSTR> controllerMap_ = {
		{5000, L"D-Up"},
		{5001, L"D-Down"},
		{5002, L"D-Right"},
		{5003, L"D-Left"},
		{5004, L"Start"},
		{5005, L"Back"},
		{5006, L"L3"},
		{5007, L"R3"},
		{5008, L"LB"},
		{5009, L"RB"},
		{5010, L"A"},
		{5011, L"B"},
		{5012, L"X"},
		{5013, L"Y"},
		{5014, L"LT"},
		{5015, L"RT"},
	};

public:
	ColorPickerWindow colorPicker;

	/**
	@brief Sets the preview colors of the color selection controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void colorHandles(LPARAM lParam) const;

	/**
	@brief Implements an inherited method responsible for handling messages sent to the window.

	@param uMsg The message to handle. should be forwarded from a message handling method.

	@param wParam should be forwarded from a message handling method.

	@param lParam should be forwarded from a message handling method.

	@return LRESULT
	*/
	LRESULT handleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam) override;

	/**
	@brief Retrieves the window's class name.
	@return LPCWSTR representing the window's class name.
	*/
	LPCWSTR className() const override { return L"Settings Window"; }
};
