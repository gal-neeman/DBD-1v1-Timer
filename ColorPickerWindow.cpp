#include <Windows.h>
#include <CommCtrl.h>
#include "Globals.h"
#include "CommCtrlUtils.h"
#include "ColorPickerWindow.h"
#include "Program.h"
#include <exception>

// Create and store the squared controls that represent selectable colors
void ColorPickerWindow::initializeColorButtons()
{
	int i = 0;
	constexpr int baseRowPos = 40;
	constexpr int baseColPos = 20;
	constexpr int buttonSize = 30;
	constexpr int offset = 35;

	for (size_t row = 0; row < 5; row++)
	{
		for (size_t col = 0; col < 5; col++)
		{

			hColorButtons_[i] = CreateWindowEx(
				0, WC_BUTTON, L"", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 
				baseRowPos + offset * col, baseColPos + offset * row, buttonSize, 
				buttonSize, hwnd_, (HMENU)i, nullptr, nullptr
			);

			i++;
		}
	}

	// Preview Color
	hPreviewColorButton_ = CreateWindowEx(
		0, WC_BUTTON, L"", 
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 105, 210, 40, 40, 
		hwnd_, (HMENU)CID_COLOR_PREVIEW, nullptr, nullptr
	);

	switch (controlId)
	{
	case CID_TIMER_COLOR:
		previewColorIndex_ = pTempSettings->colors.timerColor;
		break;
	case CID_SELECTED_TIMER_COLOR:
		previewColorIndex_ = pTempSettings->colors.selectedTimerColor;
		break;
	case CID_LAST_SECONDS_COLOR:
		previewColorIndex_ = pTempSettings->colors.lastSecondsColor;
		break;
	case CID_BACKGROUND_COLOR:
		previewColorIndex_ = pTempSettings->colors.backgroundColor;
		break;
	default:
		break;
	}
}

void ColorPickerWindow::initializeWindow()
{
	initializeColorButtons();

	// Initialize exit controls
	HWND hwndOkButton = CreateWindowEx(
		0, WC_BUTTON, L"OK", 
		WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 150,
		SIZE_COLORPICKER_HEIGHT - 70, 50, 25, hwnd_, 
		(HMENU)CID_OK, nullptr, nullptr
	);

	HWND hwndCancelButton = CreateWindowEx(
		0, WC_BUTTON, L"CANCEL", 
		WS_VISIBLE | WS_CHILDWINDOW, SIZE_COLORPICKER_WIDTH - 90, 
		SIZE_COLORPICKER_HEIGHT - 70, 70, 25, hwnd_, 
		(HMENU)CID_CANCEL, nullptr, nullptr
	);

	setControlsFont(hwnd_);
}

void ColorPickerWindow::updateSettings() const
{
	switch (controlId)
	{
	case CID_TIMER_COLOR:
		pTempSettings->colors.timerColor = previewColorIndex_;
		break;
	case CID_SELECTED_TIMER_COLOR:
		pTempSettings->colors.selectedTimerColor = previewColorIndex_;
		break;
	case CID_LAST_SECONDS_COLOR:
		pTempSettings->colors.lastSecondsColor = previewColorIndex_;
		break;
	case CID_BACKGROUND_COLOR:
		pTempSettings->colors.backgroundColor = previewColorIndex_;
		break;
	default:
		break;
	}

	RedrawWindow(GetWindow(hwnd_, GW_OWNER), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void ColorPickerWindow::handleControlCommand(const LPARAM lParam)
{
	// retrieve clicked control information
	const HWND hwndCtrl = reinterpret_cast<HWND>(lParam); // clicked item handle
	const int cid = GetDlgCtrlID(hwndCtrl); // retrieve control ID

	// clicked a color
	if (cid >= 0 && cid < PALETTE_SIZE)
	{
		previewColorIndex_ = cid;
		RedrawWindow(hwnd_, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		return;
	}

	// clicked some other control
	switch (cid)
	{
	case CID_OK:
		updateSettings();
		DestroyWindow(hwnd_);
		break;
	case CID_CANCEL:
		DestroyWindow(hwnd_);
		break;
	default:
		break;
	}
}

LRESULT ColorPickerWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
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
			hwnd_ = nullptr;
			return 0;
		case WM_COMMAND: // Control item clicked
			handleControlCommand(lParam);
			return 0;
		case WM_DRAWITEM:
		{
			const LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;
			if (pDis->CtlID == CID_COLOR_PREVIEW)
			{
				fillRectWithPaletteColor(pDis->hDC, pDis->rcItem, previewColorIndex_);
			}
			else
			{
				fillRectWithPaletteColor(pDis->hDC, pDis->rcItem, pDis->CtlID);
			}

			return 0;
		}
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