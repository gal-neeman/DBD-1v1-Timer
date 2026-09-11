#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include "BaseWindow.h"
#include "Timer.h"
#include "SettingsWindow.h"

enum MousePos : uint8_t
{
	TopLeft,
	Top,
	TopRight,
	Right,
	BottomRight,
	Bottom,
	BottomLeft,
	Left,
	None
};

// The class responsible for the main window of the app
class MainWindow : public BaseWindow<MainWindow>
{
private:
	// Resources
	ID2D1Factory* pFactory_;
	ID2D1HwndRenderTarget* pRenderTarget_;
	ID2D1SolidColorBrush* pBrushTimer_;
	ID2D1SolidColorBrush* pBrushSelectedTimer_;
	ID2D1SolidColorBrush* pBrushLastSeconds_;
	D2D1_COLOR_F backgroundColor_;
	
	// Writing Resources
	IDWriteFactory* pWriteFactory_;
	IDWriteTextFormat* pTextFormat_;

	// Fields
	Timer* activeTimer_ = &timer1;
	BOOL mouseDown_ = false;
	int clickMousePos_[2] = { 0, 0 };
	bool isResizing_ = false;
	int dir_ = -1;
	int spaceOffset_ = 8;
	
	/**
	@brief Creates the graphic resources for the main window.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT createGraphicsResources();

	/**
	@brief Creates the device independent resources for the main window.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT createDeviceIndependentResources();

	/**
	@brief Set a new font size for the timers.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT changeFontSize(float fontSize);

	/**
	@return The largest font size that can fit the window in it's current proportions.
	*/
	float getLargestFontsizeFit() const;

	/**
	@brief Dispose of graphic resources.
	*/
	void discardGraphicsResources();

	/**
	@brief Adjust the render target's size after the window has been resized.

	@return HRESULT representing the success of the operation.
	*/
	HRESULT adjustRendertargetSize() const;

	/**
	@brief Get the MousePos enum value of the mouse's direction.

	@param lParam Contains information on the mouse's state. Should be forwarded from a message handler.

	@param windowPos A RECT representing the main window's current proportions.

	@return The MousePos enum value of the mouse's direction.
	*/
	MousePos getMouseDir(LPARAM lParam, RECT windowPos) const;

	/**
	@brief The method responsible for drawing to the main window and invoking the timer to draw to it aswell.
	*/
	void handlePainting();

	/**
	@brief Handles mouse movements, including changing the cursor according to it's position in the window, 
	and detecting clicks to apply resize logic.

	@param lParam Contains information on the mouse's state and position. 
	This argument should be forwarded to this function by the message handler.
	*/
	void handleMouseMovement(LPARAM lParam) const;

	/**
	@brief Get the COLORF value of an HBRUSH type.

	@param hBrush The HBRUSH to retrieve a COLORF from.

	@return D2D1_COLOR_F value of the given HBRUSH.
	*/
	static D2D1_COLOR_F hBrushToColorf(HBRUSH hBrush);
	
	/**
	@brief Retrieve and apply the colors from the settings file to the timer brushes.
	*/
	void refreshBrushes();

public:
	// Public fields
	Timer timer1 = Timer();
	Timer timer2 = Timer();
	SettingsWindow settingsWindow;

	// Constructor
	MainWindow();

	// Prevent copying and moving of MainWindow

	MainWindow(const MainWindow& other) = delete;

	MainWindow& operator=(const MainWindow& other) = delete;

	MainWindow(const MainWindow&& other) = delete;

	MainWindow& operator=(MainWindow&& other) noexcept = delete;

	// Destructor
	~MainWindow();

	// Methods

	/**
	@brief Retrieves the window's class name.
	@return LPCWSTR representing the window's class name.
	*/
	LPCWSTR className() const override { return L"Main Window"; }

	/**
	@brief Implements an inherited method responsible for handling messages sent to the window.

	@param uMsg The message to handle. should be forwarded from a message handling method.

	@param wParam should be forwarded from a message handling method.

	@param lParam should be forwarded from a message handling method.

	@return LRESULT
	*/
	LRESULT handleMessage(UINT wMsg, WPARAM wParam, LPARAM lParam) override;

	/**
	@brief Handle hotkey inputs from the user.

	@param code The code of the hotkey that the user hit.
	*/
	void handleHotKey(int code);

	/**
	@brief Handle controller button input.

	@param buttons The buttons that had a state change.
	*/
	void handleControllerInput(WORD buttons) const;
	/**
	@brief The function that is called when a WM_PAINT event is registered to the window. This method forwards the task to handlePainting().
	*/
	void draw();
};