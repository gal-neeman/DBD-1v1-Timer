#pragma once
#include "TimerApi/Dtos/PlayerDetailsDto.h"
#include <Windows.h>
#include <winhttp.h>
#include <string>

// Messages the Timer API posts to the replyTo window. lParam is always a heap-allocated
// payload the receiving window must take ownership of.
constexpr int TIMER_API_STATUS_RESPONSE(WM_APP + 4); // StatusDto*
constexpr int TIMER_API_PLAYER_RESPONSE(WM_APP + 5); // PlayerResponse*
constexpr int TIMER_API_ERROR(WM_APP + 6);           // TimerApiFailure*

// Which request a TIMER_API_ERROR is about.
enum class TimerApiRequest : int
{
	Status,
	Player,
};

// The player id is the one requested, so a result can be matched to the player it was for.
struct PlayerResponse
{
	std::wstring playerId;
	PlayerDetailsDto details;
};

struct TimerApiFailure
{
	TimerApiRequest request;
	DWORD code; // the HTTP status, or 0 for a network failure or a body that doesn't map
	std::wstring playerId; // empty for Status
};

// winhttp.h has no constant for it.
constexpr DWORD HTTP_STATUS_TOO_MANY_REQUESTS = 429;

// Whether a failed request means the API is down. A player that doesn't exist is an answer, and
// "too many requests" means the server is up and wants us to slow down, so neither disables the API.
inline bool isApiOutage(const TimerApiRequest request, const DWORD code)
{
	const bool playerNotFound = request == TimerApiRequest::Player && code == HTTP_STATUS_NOT_FOUND;
	return !playerNotFound && code != HTTP_STATUS_TOO_MANY_REQUESTS;
}
