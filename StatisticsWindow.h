#pragma once
#include "BaseWindow.h"

// Class for the DBDLeague statistics window
class StatisticsWindow : public BaseWindow<StatisticsWindow>
{
public:
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
	LPCWSTR className() const override { return L"Statistics Window"; }
};
