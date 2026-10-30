#pragma once
#include "BaseWindow.h"

/**
@brief Create the title style font. The caller owns it and must delete it after the controls using it are destroyed.

@return The font that was created.
*/
HFONT createTitleFont();

/**
@brief Create the default controls font. The caller owns it and must delete it after the controls using it are destroyed.

@return The font that was created.
*/
HFONT createControlsFont();

/**
@brief Apply a font to a specific window's child controls.

@param hWnd The handle to the window who's child controls will be affected.

@param hFont The font to apply. The caller still owns it.
*/
void setControlsFont(const HWND hWnd, const HFONT hFont);

/**
@brief A procedure for windows of type "control", used to assign a font.

@param hControl The HWND control to assign the font to.

@param lParam An LPARAM containing the font to set for the hControl.

@return BOOL
*/
BOOL CALLBACK controlProc(const HWND hControl, const LPARAM lParam);