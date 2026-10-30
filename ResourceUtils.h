#pragma once
#include "BaseWindow.h"

// The colors that can be chosen. Settings store indexes into this palette.
// Never add pure black: it's the color key for transparency, which is why index 20 is RGB(1, 1, 1).
constexpr int PALETTE_SIZE = 25;
constexpr COLORREF PALETTE[PALETTE_SIZE] = {
	RGB(255, 0, 0), RGB(255, 77, 0), RGB(255, 116, 0), RGB(255, 154, 0), RGB(255, 193, 0),
	RGB(1, 55, 125), RGB(0, 157, 209), RGB(151, 231, 245), RGB(115, 211, 72), RGB(38, 177, 112),
	RGB(49, 0, 74), RGB(51, 0, 123), RGB(76, 0, 164), RGB(131, 0, 196), RGB(171, 0, 255),
	RGB(255, 0, 255), RGB(192, 64, 255), RGB(128, 128, 255), RGB(64, 182, 255), RGB(0, 255, 255),
	RGB(1, 1, 1), RGB(35, 35, 35), RGB(85, 85, 85), RGB(182, 176, 169), RGB(237, 231, 224)
};

template <class T> void safeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

/**
@brief Load a bitmap resource.

@param bitmap The bitmap resource to load.

@return A handle to the bitmap loaded.
*/
HBITMAP loadBitmapResource(int bitmap);

/**
@brief Fill a rectangle with a color from the palette.

@param hdc The device context to draw to.

@param rect The rectangle to fill.

@param colorIndex The index of the color in the palette.
*/
void fillRectWithPaletteColor(HDC hdc, const RECT& rect, int colorIndex);