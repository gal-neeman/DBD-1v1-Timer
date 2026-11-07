#pragma once
#include "League/Models/OverlayStats.h"
#include <optional>
#include <string>

// The main window's DBDLeague data: which player it fetches and polls, and the overlay's model.
struct LeagueState
{
	std::wstring activePlayerId;  // the player fetched and polled: the saved ID, or one Load Stats loaded since
	std::wstring pendingPlayerId; // set by Load Stats until its result arrives
	std::optional<OverlayStats> overlayStats;
	int slowPollSec = 0;		  // from the last status response
	bool playerPolling = false;	  // whether the poll timer is running
};
