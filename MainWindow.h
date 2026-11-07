#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include "BaseWindow.h"
#include "Timer.h"
#include "SettingsWindow.h"
#include "StatisticsWindow.h"
#include "League/LeagueState.h"
#include "TimerApi/Dtos/PlayerDetailsDto.h"
#include "TimerApi/Dtos/StatusDto.h"
#include "TimerApi/TimerApiMessages.h"
#include <optional>
#include <string>

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
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pBrushOverlay_; // recolored from OVERLAY_PALETTE before each use
	D2D1_COLOR_F backgroundColor_;

	// Writing Resources
	Microsoft::WRL::ComPtr<IDWriteFactory> pWriteFactory_;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pNamesFormat_; // the names and score row
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pBarFormat_;	 // the stats bar
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pBadgeFormat_; // the rank badge
	Microsoft::WRL::ComPtr<IDWriteInlineObject> pEllipsis_;	 // trims long names

	// Resize constants
	static constexpr int RESIZE_GRIP = 8; // size in pixels of the square resize area in each corner
	static constexpr float MIN_SCALE = 0.5f;
	static constexpr float MAX_SCALE = 4.0f;

	// Overlay layout, in base units (scale 1.0). TimerOnly is SIZE_MAIN_WIDTH x SIZE_MAIN_HEIGHT.
	static constexpr float OVERLAY_WIDTH = 380.0f;		// both stats layouts
	static constexpr float TIMER_ROW_HEIGHT = SIZE_MAIN_HEIGHT;
	static constexpr float STATS_BAR_HEIGHT = 24.0f;
	static constexpr float NAMES_ROW_HEIGHT = 36.0f;
	static constexpr float BADGE_WIDTH = OVERLAY_WIDTH - SIZE_MAIN_WIDTH; // the timers keep their size next to it
	static constexpr float SCORE_WIDTH = 80.0f;
	static constexpr float CELL_PADDING = 6.0f;
	static constexpr float NAMES_FONT_SIZE = 19.0f;
	static constexpr float BAR_FONT_SIZE = 17.0f;
	static constexpr float BADGE_FONT_SIZE = 22.0f;

	// The stats overlay's colors. Never add pure black: it's the color key for transparency.
	enum OverlayColor
	{
		OVERLAY_BAR_BACKGROUND,
		OVERLAY_NAME_BACKGROUND,
		OVERLAY_SCORE_BACKGROUND,
		OVERLAY_BADGE_BACKGROUND,
		OVERLAY_TEXT,
		OVERLAY_WIN, // also the winrate
		OVERLAY_LOSS,
		OVERLAY_COLOR_COUNT
	};
	static constexpr COLORREF OVERLAY_PALETTE[OVERLAY_COLOR_COUNT] = {
		RGB(40, 40, 40),	// bar background
		RGB(50, 50, 50),	// name background
		RGB(32, 32, 32),	// score background
		RGB(32, 32, 32),	// badge background
		RGB(225, 225, 225), // text
		RGB(30, 160, 95),	// win
		RGB(195, 50, 25),	// loss
	};

	// SetTimer IDs
	static constexpr UINT_PTR TIMER_ID_FRAME = 0;		  // redraws the running timers every 16 ms
	static constexpr UINT_PTR TIMER_ID_PLAYER_POLL = 1;  // fetches the active player every slowPollSec, or fastPollSec during a burst
	static constexpr UINT_PTR TIMER_ID_STATUS_RETRY = 2; // re-checks the status while the API is off
	static constexpr UINT STATUS_RETRY_MS = 3 * 60 * 1000;

	// Fields
	Timer* activeTimer_ = &timer1;
	float scale_ = 1.0f; // current window size relative to baseSize(league_.layout)
	bool inSizeMove_ = false; // the user is dragging or resizing the window; layout changes wait until they let go
	LeagueState league_; // DBDLeague player, polling and overlay data

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
	@brief The window size of a layout at scale 1.0.

	@param layout The layout.

	@return The size in base units.
	*/
	static D2D1_SIZE_F baseSize(OverlayLayout layout);

	/**
	@brief The layout the applied league settings and the loaded stats call for.

	@return TimerOnly without stats or with the overlay off, otherwise Stats or StatsWithNames.
	*/
	OverlayLayout derivedLayout() const;

	/**
	@brief Switches to the layout and resizes the window to it at the current scale, keeping the top-left corner.

	@param layout The new layout.
	*/
	void setLayout(OverlayLayout layout);

	/**
	@brief Draws the two timers, each in one half of the row.

	@param row The timers' row.
	*/
	void paintTimers(const D2D1_RECT_F& row);

	/**
	@brief Draws the player's rank next to the timers (Stats layout).

	@param rect The badge's rect.

	@param stats The overlay's stats.
	*/
	void paintRankBadge(const D2D1_RECT_F& rect, const OverlayStats& stats);

	/**
	@brief Draws the player and the opponent with their ranks, and the score centered between them.

	@param rect The row's rect.

	@param stats The overlay's stats.
	*/
	void paintNamesRow(const D2D1_RECT_F& rect, const OverlayStats& stats);

	/**
	@brief Draws the last results, the winrate and the streak.

	@param rect The bar's rect.

	@param stats The overlay's stats.
	*/
	void paintStatsBar(const D2D1_RECT_F& rect, const OverlayStats& stats);

	/**
	@brief Draws "name(#rank)" centered in the cell, cutting the name with an ellipsis so the rank always fits.

	@param cell Where to draw.

	@param name The name.

	@param rank The rank; empty draws "#?".
	*/
	void drawNameWithRank(const D2D1_RECT_F& cell, const std::wstring& name, std::optional<int> rank);

	/**
	@brief Fills a rect with an overlay color.
	*/
	void fillRect(const D2D1_RECT_F& rect, OverlayColor color);

	/**
	@brief Draws text with an overlay color, aligned by the format.
	*/
	void drawText(const std::wstring& text, IDWriteTextFormat* format, const D2D1_RECT_F& rect, OverlayColor color);

	/**
	@brief Measures the width of a single line of text, including trailing spaces.

	@return The width in base units, or 0 if it can't be measured.
	*/
	float measureText(const std::wstring& text, IDWriteTextFormat* format) const;

	/**
	@brief Get the COLORF value of a color from the palette.

	@param colorIndex The index of the color in the palette.

	@return D2D1_COLOR_F value of the given color.
	*/
	static D2D1_COLOR_F paletteToColorf(int colorIndex);

	/**
	@brief Converts a COLORREF to a D2D1_COLOR_F.
	*/
	static D2D1_COLOR_F colorRefToColorf(COLORREF color);

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

	/**
	@brief Handles a status result: if the API is enabled, stops the status retry, fetches the player
	and starts polling. Otherwise the retry keeps running.

	@param status The status result.
	*/
	void handleStatusResponse(const StatusDto& status);

	/**
	@brief Handles a player result: makes a pending Load Stats ID active and updates both views' models.
	A result for a player that is neither pending nor active is stale and ignored.

	@param response The requested player ID and its details.
	*/
	void handlePlayerResponse(const PlayerResponse& response);

	/**
	@brief Handles a failed request. A player failure that isApiOutage means the API went down:
	stops polling and restarts the status retry. A failed status needs nothing, its retry is running.

	@param failure The request that failed, its HTTP status (or 0), and the player ID it was for.
	*/
	void handleApiError(const TimerApiFailure& failure);

	// Whether a result is for the Load Stats ID still in flight.
	bool isPendingPlayer(const std::wstring& playerId) const;

	/**
	@brief Starts or stops polling: polls while the API is enabled, the overlay is enabled and a player is active.
	*/
	void updatePlayerPolling();

	/**
	@brief Starts or restarts a burst of fast polls, because a stopped timer2 means a set ended and the match
	may have too. Polls every fastPollSec for fastPollCapSec. Does nothing unless polling is on and fastPollSec > 0.
	*/
	void startFastPolling();

	/**
	@brief Ends a fast poll burst: re-arms the poll timer at the slow interval.
	*/
	void endFastPolling();

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

	/**
	@brief The ID of the player being fetched and polled. Empty if there's none.
	*/
	const std::wstring& activePlayerId() const { return league_.activePlayerId; }

	/**
	@brief Fetches a player for Load Stats. It becomes the active player once its result arrives.

	@param id A valid Discord ID.
	*/
	void loadPlayer(const std::wstring& id);

	/**
	@brief Drops a player loaded since the last save (statistics window Cancel): makes the saved ID
	active again and fetches it, or clears the models if there's no saved ID.
	*/
	void revertPlayer();

	/**
	@brief Fetches the active player once, unless polling already keeps it fresh.
	*/
	void refreshPlayerIfNotPolling();

	/**
	@brief Brings the overlay in line with the league settings and the loaded stats: switches the layout,
	starts or stops polling, and redraws. Call it after applying league settings or changing the stats.
	*/
	void refreshOverlay();
};