#pragma once

// Every fixed text the statistics window shows. Formatting characters (rank "#", "(?)", "%", the
// ellipsis) stay in StatisticsWindow.cpp.
namespace StatsText
{
	inline constexpr wchar_t WINDOW_TITLE[] = L"DBDLeague Statistics";

	// Controls
	inline constexpr wchar_t DISCORD_ID_LABEL[] = L"Your Discord ID";
	inline constexpr wchar_t LOAD_STATS_BUTTON[] = L"Load Stats";
	inline constexpr wchar_t OVERLAY_LABEL[] = L"Enable overlay stats";
	inline constexpr wchar_t SHOW_NAMES_LABEL[] = L"Show names and score";
	inline constexpr wchar_t OK_BUTTON[] = L"Ok";
	inline constexpr wchar_t CANCEL_BUTTON[] = L"Cancel";
	inline constexpr wchar_t APPLY_BUTTON[] = L"Apply";

	// State messages
	inline constexpr wchar_t DISABLED_MESSAGE[] = L"The stats feature is currently disabled"; // when the status has no message
	inline constexpr wchar_t NEEDS_ID_MESSAGE[] = L"Enter your discord id to enable this feature";
	inline constexpr wchar_t INVALID_ID_MESSAGE[] = L"Enter a valid discord id (17-20 digits)";
	inline constexpr wchar_t PLAYER_NOT_FOUND_MESSAGE[] = L"No player was found with this discord id";
	inline constexpr wchar_t LOADING_MESSAGE[] = L"Loading stats...";
	inline constexpr wchar_t ERROR_MESSAGE[] = L"Couldn't load the stats, try again later";

	// Notice line, above the stats
	inline constexpr wchar_t TOO_MANY_REQUESTS_NOTICE[] = L"Too many requests, try again in a moment";

	// Stats
	inline constexpr wchar_t STATS_TITLE_SUFFIX[] = L"'s Stats"; // after the player's name
	inline constexpr wchar_t RANK_LABEL[] = L"Rank: ";
	inline constexpr wchar_t POINTS_LABEL[] = L"Points: ";
	inline constexpr wchar_t TOTAL_GAMES_LABEL[] = L"Total games: ";
	inline constexpr wchar_t WINS_LABEL[] = L"Wins: ";
	inline constexpr wchar_t LOSSES_LABEL[] = L"Losses: ";
	inline constexpr wchar_t WINRATE_LABEL[] = L"Winrate: ";
	inline constexpr wchar_t STREAK_LABEL[] = L"Streak: ";

	// Last matches table
	inline constexpr wchar_t MATCHES_TITLE[] = L"Last Matches";
	inline constexpr wchar_t NO_MATCHES_MESSAGE[] = L"No matches played yet";
	inline constexpr wchar_t DATE_HEADER[] = L"Date";
	inline constexpr wchar_t RESULT_HEADER[] = L"Result";
	inline constexpr wchar_t SCORE_HEADER[] = L"Score";
	inline constexpr wchar_t OPPONENT_HEADER[] = L"Opponent";
	inline constexpr wchar_t POINTS_GAINED_HEADER[] = L"Points Gained";
	inline constexpr wchar_t WIN_RESULT[] = L"Win";
	inline constexpr wchar_t LOSS_RESULT[] = L"Loss";
}
