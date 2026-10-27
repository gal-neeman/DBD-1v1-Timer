#pragma once
#include "Globals.h"
#include <Windows.h>

// Template class for specific window's classes to inherit from
template <class DERIVED_CLASS> class BaseWindow
{
public:
	// public variables / methods
	BaseWindow() : hwnd_(nullptr) { }

	HWND window() const { return hwnd_; }

	// The actual window procedure every message is sent to. responsible for linking windows to class instances and forward messages to their handlers.
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_CLASS* pThis = nullptr;

		if (uMsg == WM_NCCREATE) // if a window has just been created, link it to this class instance
		{
			CREATESTRUCT* cStruct = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_CLASS*)cStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hwnd_ = hwnd;
		}
		else // if it's just another message to an existing window, save it's class instance to this local variable
		{
			pThis = (DERIVED_CLASS*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis != nullptr) // if the class instance exsists (it should) forward the message to the relevant handler method
		{
			return pThis->handleMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	// Create an HWND (with the windowproc of basewindow that will link this class instance to the HWND)
	BOOL create(
		const LPCWSTR lpWindowName,
		const int x = CW_USEDEFAULT,
		const int y = CW_USEDEFAULT,
		const int width = CW_USEDEFAULT,
		const int height = CW_USEDEFAULT,
		const DWORD dwExStyle = WS_OVERLAPPEDWINDOW,
		const DWORD dwStyle = 0,
		const HWND parent = nullptr,
		const HMENU hMenu = nullptr,
		const HINSTANCE hInstance = nullptr,
		const LPVOID lpParam = nullptr
	) {
		// create a window class
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = DERIVED_CLASS::windowProc;
		wc.lpszClassName = className();
		wc.hInstance = nullptr;
		wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		RegisterClass(&wc);

		hwnd_ = CreateWindowEx(dwExStyle, className(), lpWindowName, dwStyle, x, y, width, height, parent, hMenu, hInstance, this);

		winSize_[0] = width; winSize_[1] = height;

		return (hwnd_ ? TRUE : FALSE);
	}

protected:
	HWND hwnd_;
	int winSize_[2] = { 0, 0 };

	// protected methods to be rewritten by derived classes
	virtual LPCWSTR className() const { return nullptr; }
	virtual LRESULT handleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam) = 0;
};