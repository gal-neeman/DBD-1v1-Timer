#pragma once
#include "BaseWindow.h"
#include "League/Models/StatsWindowModel.h"
#include "TimerApi/TimerApiMessages.h"
#include "globals.h"
#include <optional>
#include <string>
#include <vector>

// What the statistics window shows below its settings.
enum class StatsState
{
	Disabled,		// the API is off; shows the status message in red
	NeedsId,		// no player loaded; asks for a Discord ID
	InvalidId,		// the typed ID isn't 17-20 digits
	PlayerNotFound, // the API has no player with that ID
	Loading,		// a player request is in flight
	Loaded,			// shows the player's stats
	Error,			// the player request failed
};

// Class for the DBDLeague statistics window.
// The main window owns the instance for the whole run and pushes API results into it, whether or
// not the window is open; the state and model outlive the HWND and are shown when it opens.
class StatisticsWindow : public BaseWindow<StatisticsWindow>
{
private:
	// The "Last Matches" table's columns, left to right
	enum MatchColumn { Date, Result, Score, Opponent, Points, COLUMN_COUNT };

	// Where each "Last Matches" column sits, from layoutMatchColumns
	struct MatchColumns
	{
		int x[COLUMN_COUNT] = {};
		int width[COLUMN_COUNT] = {};

		// The column's cell in the row starting at y
		RECT cell(MatchColumn column, int y) const;
	};

	// Survive the HWND
	StatsState state_ = StatsState::Disabled;
	std::optional<StatsWindowModel> model_;
	std::wstring statusMessage_; // from the status response; shown while Disabled
	std::wstring notice_; // a one-line notice above the stats, e.g. too many requests; cleared by the next success

	// Live only while the window is open
	LeagueSettingsStruct tempLeague_; // the checkboxes, applied on Apply or Ok
	LeagueSettingsStruct savedLeague_; // appSettings.league when the window opened; Cancel restores it
	bool applied_ = false; // whether Apply ran since the window opened
	HFONT controlsFont_ = nullptr;
	HFONT titleFont_ = nullptr;
	HFONT headerFont_ = nullptr; // the Last Matches column headers
	HBITMAP logo_ = nullptr;
	RECT contentRect_ = {}; // the painted area below the settings, redrawn on every state change
	RECT noticeRect_ = {};  // where notice_ is drawn, between the checkboxes and contentRect_

	/**
	@brief Creates the fonts, the logo and the controls, then shows the current state.
	*/
	void initializeWindow();

	/**
	@brief Creates a child control with the controls font, subclassed to turn Esc into Cancel.

	@return The control that was created.
	*/
	HWND createControl(LPCWSTR className, LPCWSTR text, int x, int y, int width, int height, int id, long additionalStyle = 0) const;

	/**
	@brief Handles WM_COMMAND from the buttons and checkboxes.

	@param controlId The ID of the control.
	*/
	void handleControlCommand(int controlId);

	/**
	@brief Switches to the given state and updates the window if it's open.

	@param state The new state.
	*/
	void setState(StatsState state);

	/**
	@brief Sets the notice line and redraws it if the window is open.

	@param notice The text to show, or empty to clear it.
	*/
	void setNotice(const std::wstring& notice);

	/**
	@brief Enables or disables the controls to match the state and the overlay checkbox.
	*/
	void updateControls() const;

	/**
	@brief Builds the league settings from the checkboxes and the active player ID.
	The overlay is off when there's no ID.

	@return The league settings to apply.
	*/
	LeagueSettingsStruct leagueFromControls() const;

	/**
	@brief Draws the logo and the content area for the current state.
	*/
	void paint() const;

	/**
	@brief Draws the Loaded state: the player's stats and the last matches table.

	@param hdc The device context to draw on.
	*/
	void paintStats(HDC hdc) const;

	/**
	@brief Draws a separator and a centered section title under it.

	@param hdc The device context to draw on.

	@param title The title.

	@param left The left edge of the section.

	@param right The right edge of the section.

	@param y The top of the section.

	@return The y below the title.
	*/
	int paintSectionTitle(HDC hdc, const std::wstring& title, int left, int right, int y) const;

	/**
	@brief Draws the 3x3 grid of the player's stats.

	@param hdc The device context to draw on.

	@param model The player's stats.

	@param left The left edge of the grid.

	@param right The right edge of the grid.

	@param y The top of the grid.

	@return The y below the grid.
	*/
	int paintStatsGrid(HDC hdc, const StatsWindowModel& model, int left, int right, int y) const;

	/**
	@brief Lays out the "Last Matches" columns at fixed widths, with the spare width split into equal gaps.

	@param left The left edge of the table.

	@param right The right edge of the table.

	@return The columns' positions and widths.
	*/
	static MatchColumns layoutMatchColumns(int left, int right);

	/**
	@brief Draws the "Last Matches" table, or a message when there are no matches.

	@param hdc The device context to draw on.

	@param matches The rows to draw.

	@param left The left edge of the table.

	@param right The right edge of the table.

	@param y The top of the table.
	*/
	void paintMatches(HDC hdc, const std::vector<MatchRow>& matches, int left, int right, int y) const;

	/**
	@brief Draws "name (#rank)" in the rect, cutting the name with an ellipsis so the rank always fits.

	@param hdc The device context to draw on.

	@param rect Where to draw.

	@param name The opponent's name.

	@param rank The opponent's rank; empty draws "(?)".
	*/
	static void drawOpponent(HDC hdc, RECT rect, const std::wstring& name, std::optional<int> rank);

	/**
	@brief Turns Esc in any control into Cancel. Installed on every control by createControl.
	*/
	static LRESULT CALLBACK ctrlWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	/**
	@brief The state to show when nothing more specific is known: Disabled while the API is off,
	Loaded with a model, Loading while main has a player to fetch, NeedsId otherwise.

	@return The derived state.
	*/
	StatsState derivedState() const;

public:
	/**
	@brief Called by the main window with every status result, after timerApiEnabled is set.

	@param message The status message from the API.
	*/
	void setStatus(const std::wstring& message);

	/**
	@brief Called by the main window when player data arrives, or with nullopt when it's cleared.

	@param model The player's stats, or nullopt.
	*/
	void setModel(std::optional<StatsWindowModel> model);

	/**
	@brief Called by the main window when a request fails.

	@param request The request that failed.

	@param code The HTTP status, or 0 for a network failure or a body that doesn't map.
	*/
	void setError(TimerApiRequest request, DWORD code);

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
