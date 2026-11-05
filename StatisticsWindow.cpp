#include <Windows.h>
#include "Globals.h"
#include "StatisticsWindow.h"
#include "Program.h"
#include <exception>

LRESULT StatisticsWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
{
	try
	{
		switch (wMsg)
		{
		case WM_DESTROY:
			hwnd_ = nullptr;
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
