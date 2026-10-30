#include "BaseWindow.h"
#include "CommCtrlUtils.h"

BOOL CALLBACK controlProc(const HWND hControl, const LPARAM lParam)
{
	HFONT hFont = (HFONT)lParam;
	SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);

	return true;
}

HFONT createTitleFont()
{
	return CreateFont(
		16, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);
}

HFONT createControlsFont()
{
	return CreateFont(
		16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
	);
}

void setControlsFont(const HWND hWnd, const HFONT hFont)
{
	EnumChildWindows(hWnd, controlProc, (LPARAM)hFont);
}