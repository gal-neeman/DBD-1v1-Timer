#pragma once
#include "TimerApi/Http/HttpWorker.h"
#include "TimerApi/TimerApiMessages.h"
#include <Windows.h>
#include <memory>
#include <string>

// The app's interface to the Timer API. Every method returns immediately; the request runs on
// the worker thread and posts exactly one message to `replyTo` (see TimerApiMessages.h).
// On success, the result is a heap-allocated DTO in lParam:
//   case TIMER_API_STATUS_RESPONSE:
//       std::unique_ptr<StatusDto> dto(reinterpret_cast<StatusDto*>(lParam)); // take ownership
// On failure, TIMER_API_ERROR is posted and timerApiEnabled is set to false, except for a player
// that doesn't exist (404) and too many requests (429), see isApiOutage. getStatus sets timerApiEnabled to the response's statusEnabled before
// posting. Only getStatus runs while it is false.
class TimerApiService
{
private:
	HttpWorker worker_;

public:
	TimerApiService();

	/**
	 * @brief Start the worker thread. Call once at startup.
	 */
	void start();

	/**
	 * @brief Abort any in-flight request and join the worker thread. Returns immediately.
	 */
	void stop();

	/**
	 * @brief GET /status. Posts TIMER_API_STATUS_RESPONSE with a StatusDto* and sets timerApiEnabled
	 * to its statusEnabled, or posts TIMER_API_ERROR.
	 */
	void getStatus(HWND replyTo);

	/**
	 * @brief GET /players/<playerId>. Posts TIMER_API_PLAYER_RESPONSE with a PlayerDetailsDto*,
	 * or TIMER_API_ERROR (404 if the player doesn't exist). Fails at once while timerApiEnabled is false.
	 */
	void getPlayerDetails(const std::wstring& playerId, HWND replyTo);

private:
	// Prepends the API base path, e.g. L"/status" -> L"/api/v1/status".
	static std::wstring endpoint(const std::wstring& path);

	static bool isSuccess(const HttpResponse& response);

	// Posts TIMER_API_ERROR and sets timerApiEnabled to false, unless the player wasn't found.
	static void fail(HWND replyTo, TimerApiRequest request, DWORD code);

	// Queues the job and turns any exception it throws, or a worker that isn't running, into fail(0).
	template <typename Job>
	void enqueueRequest(HWND replyTo, TimerApiRequest request, Job job);

	// Posts the DTO to the window, which takes ownership. Deletes it if the window is gone.
	template <typename Dto>
	static void postResult(HWND replyTo, UINT message, std::unique_ptr<Dto> dto);
};

// The single, app-wide service. Started and stopped in wWinMain.
extern TimerApiService timerApi;
