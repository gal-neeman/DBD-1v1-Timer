#pragma once
#include <optional>
#include <string>
#include <vector>

// One row of the "Last Matches" table, formatted for display.
struct MatchRow
{
	std::wstring date;		   // local time, e.g. L"23/10/2026 12:02"
	bool isWin = false;
	std::wstring score;		   // e.g. L"2 - 1", or L"-" if unknown
	std::wstring opponentName;
	std::optional<int> opponentRank; // empty when unranked
	std::wstring pointsGained; // 5 characters including the sign, e.g. L"+12.0"
	bool pointsPositive = false;
};

// What the statistics window shows. Rank is empty when unranked.
struct StatsWindowModel
{
	std::wstring playerName;
	std::optional<int> rank;
	std::wstring points; // 6 characters, e.g. L"1234.5"
	int totalGames = 0;
	int wins = 0;
	int losses = 0;
	int winrate = 0; // percent
	int streak = 0;
	std::vector<MatchRow> matches; // up to 10, newest first
};
