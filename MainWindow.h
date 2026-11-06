#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include "BaseWindow.h"
#include "Timer.h"
#include "SettingsWindow.h"
#include "StatisticsWindow.h"

// The class responsible for the main window of the app
class MainWindow : public BaseWindow<MainWindow>
{
private:
	// Resources
	Microsoft::WRL::ComPtr<ID2D1Factory> pFactory_;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> pRenderTarget_;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pBrushTimer_;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pBrushSelectedTimer_;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pBrushLastSeconds_;
	D2D1_COLOR_F backgroundColor_;

	// Writing Resources
	Microsoft::WRL::ComPtr<IDWriteFactory> pWriteFactory_;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;

	// Resize constants
	static constexpr int RESIZE_GRIP = 8; // size in pixels of the square resize area in each corner
	static constexpr float MIN_SCALE = 0.5f;
	static constexpr float MAX_SCALE = 4.0f;

	// Fields
	Timer* activeTimer_ = &timer1;
	float scale_ = 1.0f; // current window size relative to SIZE_MAIN_WIDTH x SIZE_MAIN_HEIGHT

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
	@brief Dispose of graphic resources.
	*/
	void discardGraphicsResources();

	/**
	@brief Handles WM_NCHITTEST: the corners resize the window, everywhere else is client area.

	@param lParam The cursor position in screen coordinates, forwarded from WM_NCHITTEST.

	@return The HT* code for the cursor position.
	*/
	LRESULT hitTest(LPARAM lParam) const;

	/**
	@brief Handles WM_SIZING: locks the window to the base aspect ratio and the scale limits,
	keeping the corner opposite to the dragged one in place.

	@param edge The WMSZ_* edge being dragged, forwarded from WM_SIZING.

	@param rect The proposed window rect, adjusted in place.
	*/
	void constrainSizingRect(WPARAM edge, RECT* rect) const;

	/**
	@brief Handles WM_SIZE: updates the scale and resizes the render target.

	@param width The new client width.

	@param height The new client height.
	*/
	void handleResize(UINT width, UINT height);

	/**
	@brief The method responsible for drawing to the main window and invoking the timer to draw to it aswell.
	*/
	void handlePainting();

	/**
	@brief Get the COLORF value of a color from the palette.

	@param colorIndex The index of the color in the palette.

	@return D2D1_COLOR_F value of the given color.
	*/
	static D2D1_COLOR_F paletteToColorf(int colorIndex);
	
	/**
	@brief Retrieve and apply the colors from the settings file to the timer brushes.
	*/
	void refreshBrushes();

	/**
	@brief return a brush for timer 2. if timer 1 is less than optionLastSecondsTime away from timer 2, use a special brush. otherwise use the normal one.

	@return ID2D1SolidColorBrush* A pointer to the brush to apply.
	*/
	ID2D1SolidColorBrush* getTimer2Brush() const;

	/**
	@brief Calling this will cause a WM_PAINT message to be queued for it, which will handle a re-painting of the window
	*/
	void requestRedraw() const;

public:
	// Public fields
	Timer timer1 = Timer();
	Timer timer2 = Timer();
	SettingsWindow settingsWindow;
	StatisticsWindow statisticsWindow;

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
	@brief Apply the window related settings (transparency and click through) to the main window.

	@param settings The settings to apply.
	*/
	void applyWindowSettings(const SettingsStruct& settings) const;
};