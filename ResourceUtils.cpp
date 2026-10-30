#include "ResourceUtils.h"

HBITMAP loadBitmapResource(const int bitmap) {
	const HBITMAP hBitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(bitmap));
	if (!hBitmap) {
		MessageBox(nullptr, L"Failed to load bitmap!", L"Error", MB_OK);
	}

	return hBitmap;
}

void fillRectWithPaletteColor(const HDC hdc, const RECT& rect, const int colorIndex)
{
	SetDCBrushColor(hdc, PALETTE[colorIndex]);
	FillRect(hdc, &rect, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
}