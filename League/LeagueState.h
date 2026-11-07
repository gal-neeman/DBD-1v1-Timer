#pragma once
#include "League/Models/OverlayStats.h"
#include <optional>
#include <string>

// What the main window shows, from the applied league settings and whether stats are loaded.
enum class OverlayLayout
{
	TimerOnly,		// the overlay is off or there are no stats
	Stats,			// the timers, the rank badge and the stats bar
	StatsWithNames, // the names and score row, the timers and the stats bar
};

// The main window's DBDLeague data: which player it fetches and polls, and the overlay's model.
struct LeagueState
{
	std::wstring activePlayerId;  // the player fetched and polled: the saved ID, or one Load Stats loaded since
	std::wstring pendingPlayerId; // set by Load Stats until its result arrives
	std::optional<OverlayStats> overlayStats;
	OverlayLayout layout = OverlayLayout::TimerOnly;
	int slowPollSec = 0;		  // from the last status response
	bool playerPolling = false;	  // whether the poll timer is running
};
