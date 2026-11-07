#include "TimerApi/TimerApiService.h"
#include "TimerApi/Mappers/DtoMappers.h"
#include "TimerApi/TimerApiMessages.h"
#include "globals.h"

namespace
{
	// Host name only, no scheme: HTTPS is the HttpClient default.
	// Release builds (NDEBUG) use production; Debug builds use staging.
#ifdef NDEBUG
	constexpr WCHAR TIMER_API_HOST[] = L"api.example.com"; // TODO: set the production API host
#else
	constexpr WCHAR TIMER_API_HOST[] = L"dbd1v1-api-staging.truuehsoft.com";
#endif
	constexpr WCHAR TIMER_API_BASE_PATH[] = L"/api/v1";
}

TimerApiService timerApi;

TimerApiService::TimerApiService()
	: worker_(TIMER_API_HOST)
{
}

void TimerApiService::start()
{
	worker_.start();
}

void TimerApiService::stop()
{
	worker_.stop();
}

void TimerApiService::getStatus(HWND replyTo)
{
	enqueueRequest(replyTo, TimerApiRequest::Status, [replyTo](HttpClient& client)
	{
		const HttpResponse response = client.get(endpoint(L"/status"));
		if (!isSuccess(response))
		{
			fail(replyTo, TimerApiRequest::Status, response.status);
			return;
		}
		std::unique_ptr<StatusDto> status = std::make_unique<StatusDto>(mapStatus(response.body));
		timerApiEnabled = status->statusEnabled;
		postResult(replyTo, TIMER_API_STATUS_RESPONSE, std::move(status));
	});
}

void TimerApiService::getPlayerDetails(const std::wstring& playerId, HWND replyTo)
{
	if (!timerApiEnabled)
	{
		fail(replyTo, TimerApiRequest::Player, 0);
		return;
	}

	enqueueRequest(replyTo, TimerApiRequest::Player, [playerId, replyTo](HttpClient& client)
	{
		const HttpResponse response = client.get(endpoint(L"/players/" + playerId));
		if (!isSuccess(response))
		{
			fail(replyTo, TimerApiRequest::Player, response.status);
			return;
		}
		postResult(replyTo, TIMER_API_PLAYER_RESPONSE, std::make_unique<PlayerDetailsDto>(mapPlayerDetails(response.body)));
	});
}

std::wstring TimerApiService::endpoint(const std::wstring& path)
{
	return TIMER_API_BASE_PATH + path;
}

bool TimerApiService::isSuccess(const HttpResponse& response)
{
	return response.status >= 200 && response.status < 300;
}

void TimerApiService::fail(HWND replyTo, const TimerApiRequest request, const DWORD code)
{
	if (isApiOutage(request, code)) timerApiEnabled = false;
	PostMessage(replyTo, TIMER_API_ERROR, code, static_cast<LPARAM>(request));
}

template <typename Job>
void TimerApiService::enqueueRequest(HWND replyTo, const TimerApiRequest request, Job job)
{
	const bool queued = worker_.enqueue([replyTo, request, job = std::move(job)](HttpClient& client)
	{
		// Network failures, bodies that don't map, and the cancellation from stop() all end up here.
		// Posting during shutdown is harmless: the message loop has already exited.
		try
		{
			job(client);
		}
		catch (const std::exception&)
		{
			fail(replyTo, request, 0);
		}
	});

	if (!queued) fail(replyTo, request, 0);
}

template <typename Dto>
void TimerApiService::postResult(HWND replyTo, const UINT message, std::unique_ptr<Dto> dto)
{
	// On success the window owns the DTO; otherwise unique_ptr frees it here.
	if (PostMessage(replyTo, message, 0, reinterpret_cast<LPARAM>(dto.get())))
	{
		(void)dto.release();
	}
}
