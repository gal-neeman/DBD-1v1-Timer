#pragma once
#include <optional>
#include <string>

// What the stats overlay shows around the timer. Ranks are empty when unranked.
struct OverlayStats
{
	std::wstring playerName;
	std::optional<int> playerRank;
	// Only set while the player is in a match.
	std::optional<std::wstring> opponentName;
	std::optional<int> opponentRank;
	std::wstring lastResults; // up to the last 8 matches, newest first, e.g. L"W L W"
	int winrate = 0;		  // percent
	int streak = 0;
};
