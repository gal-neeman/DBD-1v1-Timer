#pragma once
#include "BaseWindow.h"
#include "League/Models/StatsWindowModel.h"
#include "TimerApi/TimerApiMessages.h"
#include <optional>
#include <string>

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
	StatsState state_ = StatsState::Disabled;
	std::optional<StatsWindowModel> model_;
	std::wstring statusMessage_; // from the status response; shown while Disabled

	/**
	@brief Switches to the given state and updates the window if it's open.

	@param state The new state.
	*/
	void setState(StatsState state);

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
