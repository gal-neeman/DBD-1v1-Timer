#include "TimerApi/Mappers/DtoMappers.h"
#include "TimerApi/Mappers/JsonHelpers.h"

namespace
{
	CurrentMatchDto mapCurrentMatch(const Json::Value& json)
	{
		CurrentMatchDto dto;
		dto.isInMatch = readBool(json, "isInMatch");
		dto.opponentName = readOptionalString(json, "opponentName");
		dto.opponentRank = readOptionalInt(json, "opponentRank");
		return dto;
	}

	MatchDto mapMatch(const Json::Value& json)
	{
		MatchDto dto;
		dto.opponentName = readString(json, "opponentName");
		dto.pointsGained = readDouble(json, "pointsGained");
		dto.matchDate = readString(json, "matchDate");
		dto.isWin = readBool(json, "isWin");
		dto.playerScore = readOptionalInt(json, "playerScore");
		dto.opponentScore = readOptionalInt(json, "opponentScore");
		dto.playerRankBefore = readInt(json, "playerRankBefore");
		dto.opponentRankBefore = readInt(json, "opponentRankBefore");
		return dto;
	}
}

StatusDto mapStatus(const std::string& body)
{
	const Json::Value json = parseJson(body);

	StatusDto dto;
	dto.statusEnabled = readBool(json, "statusEnabled");
	dto.fastPollSec = readInt(json, "fastPollSec");
	dto.slowPollSec = readInt(json, "slowPollSec");
	dto.fastPollCapSec = readInt(json, "fastPollCapSec");
	dto.latestVersion = readString(json, "latestVersion");
	dto.minSupportedVersion = readString(json, "minSupportedVersion");
	dto.statusMessage = readString(json, "statusMessage");
	return dto;
}

PlayerDetailsDto mapPlayerDetails(const std::string& body)
{
	const Json::Value json = parseJson(body);

	PlayerDetailsDto dto;
	dto.playerName = readString(json, "playerName");
	dto.seasonWinrate = readInt(json, "seasonWinrate");
	dto.totalGames = readInt(json, "totalGames");
	dto.totalGamesWon = readInt(json, "totalGamesWon");
	dto.totalGamesLost = readInt(json, "totalGamesLost");
	dto.currentStreak = readInt(json, "currentStreak");
	dto.currentRank = readInt(json, "currentRank");
	dto.currentPoints = readDouble(json, "currentPoints");
	dto.currentMatch = mapCurrentMatch(readObject(json, "currentMatch"));

	for (const Json::Value& match : readArray(json, "lastMatches"))
	{
		dto.lastMatches.push_back(mapMatch(match));
	}
	return dto;
}
