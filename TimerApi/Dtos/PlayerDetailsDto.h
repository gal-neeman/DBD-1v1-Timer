#pragma once
#include <optional>
#include <string>
#include <vector>

// The player's current match. Opponent fields are empty unless isInMatch;
// searching for an opponent isn't a match.
struct CurrentMatchDto
{
	bool isInMatch = false;
	std::optional<std::wstring> opponentName;
	std::optional<int> opponentRank; // -1 if unranked or the lookup failed
};

// One match from the player's point of view.
struct MatchDto
{
	std::wstring opponentName;
	double pointsGained = 0; // negative on a loss
	std::wstring matchDate;  // ISO 8601, e.g. "2026-09-27T20:33:54.454000Z"
	bool isWin = false;
	// Games won in the set (best of 3); empty if Slugbot didn't report a score.
	std::optional<int> playerScore;
	std::optional<int> opponentScore;
	int playerRankBefore = 0;
	int opponentRankBefore = 0;
};

// Response of GET /api/v1/players/<id>.
struct PlayerDetailsDto
{
	std::wstring playerName;
	int seasonWinrate = 0;
	int totalGames = 0;
	int totalGamesWon = 0;
	int totalGamesLost = 0;
	int currentStreak = 0;
	int currentRank = 0;
	double currentPoints = 0;
	CurrentMatchDto currentMatch;
	std::vector<MatchDto> lastMatches;
};
