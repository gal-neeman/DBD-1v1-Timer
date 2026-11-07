#pragma once
#include <optional>
#include <string>
#include <vector>

// What the stats overlay shows around the timer. Ranks are empty when unranked.
struct OverlayStats
{
	std::wstring playerName;
	std::optional<int> playerRank;
	// Only set while the player is in a match.
	std::optional<std::wstring> opponentName;
	std::optional<int> opponentRank;
	std::vector<bool> lastResults; // up to the last 8 matches, newest first; true is a win
	int winrate = 0;			   // percent
	int streak = 0;
	// The set score, kept by the user with hotkeys rather than from the API
	int scorePlayer = 0;
	int scoreOpponent = 0;
};
