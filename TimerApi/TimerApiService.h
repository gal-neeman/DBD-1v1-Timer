#pragma once
#include "TimerApi/Http/HttpWorker.h"
#include <Windows.h>
#include <memory>
#include <string>

// The app's interface to the Timer API. Every method returns immediately; the request runs on
// the worker thread and the result is posted to `replyTo` as a heap-allocated DTO in lParam:
//   case TIMER_API_STATUS_RESPONSE:
//       std::unique_ptr<StatusDto> dto(reinterpret_cast<StatusDto*>(lParam)); // take ownership
// getStatus sets timerApiEnabled to the response's statusEnabled before posting. If a request
// fails, it is set to false and nothing is posted. Only getStatus runs while it is false.
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
	 * @brief GET /status. Posts TIMER_API_STATUS_RESPONSE with a StatusDto* and sets timerApiEnabled to its statusEnabled.
	 */
	void getStatus(HWND replyTo);

	/**
	 * @brief GET /players/<playerId>. Posts TIMER_API_PLAYER_RESPONSE with a PlayerDetailsDto*,
	 * or with lParam = 0 if the player doesn't exist. Ignored while timerApiEnabled is false.
	 */
	void getPlayerDetails(const std::wstring& playerId, HWND replyTo);

private:
	// Prepends the API base path, e.g. L"/status" -> L"/api/v1/status".
	static std::wstring endpoint(const std::wstring& path);

	// Throws TimerApiError unless the status is 2xx.
	static void ensureSuccess(const HttpResponse& response);

	// Posts the DTO to the window, which takes ownership. Deletes it if the window is gone.
	template <typename Dto>
	static void postResult(HWND replyTo, UINT message, std::unique_ptr<Dto> dto);
};

// The single, app-wide service. Started and stopped in wWinMain.
extern TimerApiService timerApi;
