#include "TimerApi/TimerApiService.h"
#include "TimerApi/Mappers/DtoMappers.h"
#include "TimerApi/Mappers/JsonHelpers.h"
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
	worker_.enqueue([replyTo](HttpClient& client)
	{
		const HttpResponse response = client.get(endpoint(L"/status"));
		ensureSuccess(response);
		std::unique_ptr<StatusDto> status = std::make_unique<StatusDto>(mapStatus(response.body));
		timerApiEnabled = status->statusEnabled;
		postResult(replyTo, TIMER_API_STATUS_RESPONSE, std::move(status));
	});
}

void TimerApiService::getPlayerDetails(const std::wstring& playerId, HWND replyTo)
{
	if (!timerApiEnabled) return;

	worker_.enqueue([playerId, replyTo](HttpClient& client)
	{
		const HttpResponse response = client.get(endpoint(L"/players/" + playerId));
		if (response.status == HTTP_STATUS_NOT_FOUND)
		{
			postResult<PlayerDetailsDto>(replyTo, TIMER_API_PLAYER_RESPONSE, nullptr);
			return;
		}
		ensureSuccess(response);
		postResult(replyTo, TIMER_API_PLAYER_RESPONSE, std::make_unique<PlayerDetailsDto>(mapPlayerDetails(response.body)));
	});
}

std::wstring TimerApiService::endpoint(const std::wstring& path)
{
	return TIMER_API_BASE_PATH + path;
}

void TimerApiService::ensureSuccess(const HttpResponse& response)
{
	if (response.status < 200 || response.status >= 300)
	{
		throw TimerApiError("unexpected HTTP status " + std::to_string(response.status));
	}
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
