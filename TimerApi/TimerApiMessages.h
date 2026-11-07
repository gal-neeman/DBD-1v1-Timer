#pragma once
#include <Windows.h>
#include <winhttp.h>

// Messages the Timer API posts to the replyTo window.

// Successful results. lParam is a heap-allocated DTO the receiving window must take ownership of.
constexpr int TIMER_API_STATUS_RESPONSE(WM_APP + 4); // StatusDto*
constexpr int TIMER_API_PLAYER_RESPONSE(WM_APP + 5); // PlayerDetailsDto*

// A failed request. wParam is the HTTP status, or 0 for a network failure or a body that
// doesn't map. lParam is the TimerApiRequest that failed. Nothing is allocated.
constexpr int TIMER_API_ERROR(WM_APP + 6);

// Which request a TIMER_API_ERROR is about.
enum class TimerApiRequest : int
{
	Status,
	Player,
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
