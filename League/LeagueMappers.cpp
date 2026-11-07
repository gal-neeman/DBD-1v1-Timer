#include "League/LeagueMappers.h"
#include <Windows.h>
#include <algorithm>
#include <cstdio>

namespace
{
	constexpr size_t OVERLAY_RESULTS_COUNT = 8;
	constexpr size_t WINDOW_MATCHES_COUNT = 10;
	constexpr size_t POINTS_WIDTH = 6;
	constexpr size_t POINTS_GAINED_WIDTH = 5; // including the sign

	// The API sends -1 for an unranked player.
	std::optional<int> rankOrNull(const int rank)
	{
		return rank == -1 ? std::nullopt : std::optional<int>(rank);
	}

	// Formats the value with as many decimals as fit in `width` characters, e.g. 1234.5 -> L"1234.5",
	// 12 -> L"12.000", or L"+12.00" with `sign`. Values too large for `width` come out longer.
	std::wstring formatFixedWidth(const double value, const size_t width, const bool sign)
	{
		const wchar_t* format = sign ? L"%+.*f" : L"%.*f";
		wchar_t buffer[64];
		for (int decimals = static_cast<int>(width) - 2; decimals > 0; --decimals)
		{
			swprintf_s(buffer, format, decimals, value);
			if (wcslen(buffer) <= width) return buffer;
		}
		swprintf_s(buffer, format, 0, value);
		return buffer;
	}

	// ISO 8601 UTC (e.g. "2026-09-27T20:33:54.454000Z") to local L"dd/MM/yyyy HH:mm", or L"-" if it doesn't parse.
	std::wstring formatLocalDate(const std::wstring& iso)
	{
		SYSTEMTIME utc = {};
		const int fields = swscanf_s(iso.c_str(), L"%4hu-%2hu-%2huT%2hu:%2hu:%2hu",
			&utc.wYear, &utc.wMonth, &utc.wDay, &utc.wHour, &utc.wMinute, &utc.wSecond);

		SYSTEMTIME local;
		if (fields != 6 || !SystemTimeToTzSpecificLocalTime(nullptr, &utc, &local))
		{
			return L"-";
		}

		wchar_t buffer[32];
		swprintf_s(buffer, L"%02u/%02u/%04u %02u:%02u", local.wDay, local.wMonth, local.wYear, local.wHour, local.wMinute);
		return buffer;
	}

	std::wstring formatScore(const MatchDto& match)
	{
		if (!match.playerScore || !match.opponentScore) return L"-";
		return std::to_wstring(*match.playerScore) + L" - " + std::to_wstring(*match.opponentScore);
	}
}

OverlayStats toOverlayStats(const PlayerDetailsDto& dto)
{
	OverlayStats stats;
	stats.playerName = dto.playerName;
	stats.playerRank = rankOrNull(dto.currentRank);
	stats.winrate = dto.seasonWinrate;
	stats.streak = dto.currentStreak;

	if (dto.currentMatch.isInMatch)
	{
		stats.opponentName = dto.currentMatch.opponentName;
		if (dto.currentMatch.opponentRank) stats.opponentRank = rankOrNull(*dto.currentMatch.opponentRank);
	}

	const size_t count = (std::min)(dto.lastMatches.size(), OVERLAY_RESULTS_COUNT);
	for (size_t i = 0; i < count; ++i)
	{
		stats.lastResults.push_back(dto.lastMatches[i].isWin);
	}
	return stats;
}

StatsWindowModel toStatsWindowModel(const PlayerDetailsDto& dto)
{
	StatsWindowModel model;
	model.playerName = dto.playerName;
	model.rank = rankOrNull(dto.currentRank);
	model.points = formatFixedWidth(dto.currentPoints, POINTS_WIDTH, false);
	model.totalGames = dto.totalGames;
	model.wins = dto.totalGamesWon;
	model.losses = dto.totalGamesLost;
	model.winrate = dto.seasonWinrate;
	model.streak = dto.currentStreak;
	model.inMatch = dto.currentMatch.isInMatch;

	const size_t count = (std::min)(dto.lastMatches.size(), WINDOW_MATCHES_COUNT);
	for (size_t i = 0; i < count; ++i)
	{
		const MatchDto& match = dto.lastMatches[i];
		MatchRow row;
		row.date = formatLocalDate(match.matchDate);
		row.isWin = match.isWin;
		row.score = formatScore(match);
		row.opponentName = match.opponentName;
		row.opponentRank = rankOrNull(match.opponentRankBefore);
		row.pointsGained = formatFixedWidth(match.pointsGained, POINTS_GAINED_WIDTH, true);
		row.pointsPositive = match.pointsGained >= 0;
		model.matches.push_back(std::move(row));
		(match.isWin ? model.recentWins : model.recentLosses)++;
	}
	return model;
}
