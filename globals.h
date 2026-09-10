#pragma once
#include <string>
#include <Windows.h>

// HWND Control IDs
constexpr byte CID_OK = 100;
constexpr byte CID_CANCEL = 101;
constexpr byte CID_START = 103;
constexpr byte CID_TIMER1 = 104;
constexpr byte CID_TIMER2 = 105;
constexpr byte CID_START_NO_RESET = 116;
constexpr byte CID_TRANSPARENT_CB = 106;
constexpr byte CID_CLICKTHROUGH_CB = 107;
constexpr byte CID_STARTONCHANGE_CB = 112;
constexpr byte CID_CON_START = 113;
constexpr byte CID_CON_TIMER1 = 114;
constexpr byte CID_CON_TIMER2 = 115;
constexpr byte CID_CON_START_NO_RESET = 117;
constexpr byte CID_LAST_SECONDS_TIME = 118;
constexpr byte MENU_QUIT = 1;
constexpr byte MENU_SETTINGS = 0;
constexpr byte KEY_START = 0;
constexpr byte KEY_START_NO_RESET = 5;
constexpr byte KEY_TIMER1 = 1;
constexpr byte KEY_TIMER2 = 2;
constexpr byte OPTION_TRANSPARENT = 3;
constexpr byte OPTION_CLICKTHROUGH = 4;

// HWND Color Control IDs
constexpr byte CID_TIMER_COLOR = 108;
constexpr byte CID_SELECTED_TIMER_COLOR = 109;
constexpr byte CID_LAST_SECONDS_COLOR = 110;
constexpr byte CID_BACKGROUND_COLOR = 111;
constexpr byte CID_COLOR_PREVIEW = 25;

// Controller input Buttons
constexpr USHORT CONTROLLER_UP = 5000;
constexpr USHORT CONTROLLER_DOWN = 5001;
constexpr USHORT CONTROLLER_RIGHT = 5002;
constexpr USHORT CONTROLLER_LEFT = 5003;
constexpr USHORT CONTROLLER_START = 5004;
constexpr USHORT CONTROLLER_BACK = 5005;
constexpr USHORT CONTROLLER_LEFT_THUMB = 5006;
constexpr USHORT CONTROLLER_RIGHT_THUMB = 5007;
constexpr USHORT CONTROLLER_LEFT_SHOULDER = 5008;
constexpr USHORT CONTROLLER_RIGHT_SHOULDER = 5009;
constexpr USHORT CONTROLLER_A = 5010;
constexpr USHORT CONTROLLER_B = 5011;
constexpr USHORT CONTROLLER_X = 5012;
constexpr USHORT CONTROLLER_Y = 5013;
constexpr USHORT CONTROLLER_LEFT_TRIGGER = 5014;
constexpr USHORT CONTROLLER_RIGHT_TRIGGER = 5015;

// Playstation controller input buttons
constexpr USHORT PS_CONTROLLER_START = 16;
constexpr USHORT PS_CONTROLLER_BACK = 32;
constexpr USHORT PS_CONTROLLER_L3= 64;
constexpr USHORT PS_CONTROLLER_R3 = 128;
constexpr USHORT PS_CONTROLLER_LB = 256;
constexpr USHORT PS_CONTROLLER_RB = 512;
constexpr USHORT PS_CONTROLLER_A = 4096;
constexpr USHORT PS_CONTROLLER_B = 8192;
constexpr USHORT PS_CONTROLLER_X = 16384;
constexpr USHORT PS_CONTROLLER_Y = 32768;

// Global Sizes
constexpr UINT16 SIZE_SETTINGS_WIDTH = 400;
constexpr UINT16 SIZE_SETTINGS_HEIGHT = 660;
constexpr UINT16 SIZE_COLORPICKER_WIDTH = 270;
constexpr UINT16 SIZE_COLORPICKER_HEIGHT = 350;

// Bitmaps
constexpr byte IDB_MOUSE = 110;
constexpr byte IDB_CONTROLLER = 111;

// Custom HWND messages
constexpr int REFRESH_BRUSHES(WM_APP + 1);
constexpr int HOTKEY_HIT(WM_APP + 2);
constexpr int CONTROLLER_INPUT(WM_APP + 3);

// Configuration File Names
#define SETTINGS_FILE_NAME "Settings.json"

// Structs
struct ColorsStruct // With default values
{
	int timerColor = 9;
	int selectedTimerColor = 6;
	int lastSecondsColor = 1;
	int backgroundColor = 20;
};

struct SettingsStruct // With default values
{
	int startKey = 70;
	int timer1Key = 112;
	int timer2Key = 113;
	int startNoResetKey = 72;
	int conStartKey = CONTROLLER_A;
	int conTimer1Key = CONTROLLER_LEFT;
	int conTimer2Key = CONTROLLER_RIGHT;
	int conStartNoResetKey = CONTROLLER_B;
	bool optionStartOnChange = false;
	bool optionTransparent = false;
	bool optionClickThrough = false;
	int optionLastSecondsTime = 20;
	ColorsStruct colors;
};

// Variables
extern HINSTANCE hInstanceGlobal;
extern HBRUSH hBrushes[25];
extern SettingsStruct appSettings;
extern HWND hwndMainWindow;
