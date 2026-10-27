#pragma once
#include "BaseWindow.h"

// Class for the color picker window
class ColorPickerWindow : public BaseWindow<ColorPickerWindow>
{
private:
	// Variables
	HWND hColorButtons_[25] = {};
	HBRUSH hPreviewColor_ = nullptr;
	HWND hPreviewColorButton_ = nullptr;
	int previewColorIndex_ = NULL;

	/**
	@brief Creates and stores the squared controls that represent selectable colors.
	*/
	void initializeColorButtons();

	/**
	@brief Calls required methods to initialize the settings window.
	*/
	void initializeWindow();

	/**
	@brief Apply changes to the temporary struct the settings window is using
	*/
	void updateSettings() const;

	/**
	@brief Method that handles interaction with controls.

	@param lParam should be forwarded from a message handling method.
	*/
	void handleControlCommand(LPARAM lParam);

public:
	// public variables
	int controlId; // ID of the control that summoned the color picker window
	SettingsStruct* pTempSettings; // Reference to the settings window's settings struct

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
	LPCWSTR className() const override { return L"Color Picker Window"; }
};