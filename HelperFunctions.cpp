#include "HelperFunctions.h"
#include <json/json.h>
#include <fstream>
#include <Windows.h>

using namespace std;

// Methods
// SettingsUtils
// CommUtils
BOOL CALLBACK controlProc(const HWND hControl, const LPARAM lParam)
{
	HFONT hFont = (HFONT)lParam;
	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);

	return true;
}

void setTitleFont(const HWND hControl)
{
	const HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void setControlsFont(const HWND hWnd)
{
	const HFONT hFont = CreateFont(
		16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);

	EnumChildWindows(hWnd, controlProc, (LPARAM)hFont);
}