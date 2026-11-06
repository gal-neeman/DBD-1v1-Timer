#pragma once
#include <Windows.h>

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
