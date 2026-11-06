#include "TimerApi/Http/HttpClient.h"

namespace
{
	constexpr WCHAR USER_AGENT[] = L"DBD1v1Timer";
	constexpr WCHAR JSON_CONTENT_TYPE_HEADER[] = L"Content-Type: application/json\r\n";
}

HttpError::HttpError(const std::string& operation, const DWORD code)
	: std::runtime_error(operation + " failed with error " + std::to_string(code)), code_(code)
{
}

HttpClient::HttpClient(const std::wstring& host, const INTERNET_PORT port, const bool https)
	: https_(https)
{
	session_.reset(WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
	if (!session_) throw HttpError("WinHttpOpen", GetLastError());

	if (!WinHttpSetTimeouts(session_.get(), HTTP_RESOLVE_TIMEOUT_MS, HTTP_CONNECT_TIMEOUT_MS,
		HTTP_SEND_TIMEOUT_MS, HTTP_RECEIVE_TIMEOUT_MS))
	{
		throw HttpError("WinHttpSetTimeouts", GetLastError());
	}

	connection_.reset(WinHttpConnect(session_.get(), host.c_str(), port, 0));
	if (!connection_) throw HttpError("WinHttpConnect", GetLastError());
}

HttpResponse HttpClient::request(const wchar_t* method, const std::wstring& path, const std::string& body)
{
	HINTERNET request;
	{
		std::lock_guard lock(mutex_);
		if (cancelled_) throw HttpError("request", ERROR_WINHTTP_OPERATION_CANCELLED);

		request = WinHttpOpenRequest(connection_.get(), method, path.c_str(), nullptr,
			WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, https_ ? WINHTTP_FLAG_SECURE : 0);
		if (!request) throw HttpError("WinHttpOpenRequest", GetLastError());

		activeRequest_ = request;
	}

	// Close the request handle on every exit path, unless cancel() already closed it.
	// If cancel() closes it mid-call, the blocked WinHTTP call fails and we throw below.
	struct RequestCloser
	{
		HttpClient& client;
		~RequestCloser()
		{
			std::lock_guard lock(client.mutex_);
			client.closeActiveRequestLocked();
		}
	} closer{ *this };

	const bool hasBody = !body.empty();
	const DWORD bodySize = static_cast<DWORD>(body.size());
	if (!WinHttpSendRequest(request,
		hasBody ? JSON_CONTENT_TYPE_HEADER : WINHTTP_NO_ADDITIONAL_HEADERS, hasBody ? -1L : 0,
		hasBody ? const_cast<char*>(body.data()) : WINHTTP_NO_REQUEST_DATA, bodySize, bodySize, 0))
	{
		throw HttpError("WinHttpSendRequest", GetLastError());
	}

	if (!WinHttpReceiveResponse(request, nullptr)) throw HttpError("WinHttpReceiveResponse", GetLastError());

	HttpResponse response;
	DWORD statusSize = sizeof(response.status);
	if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, &response.status, &statusSize, WINHTTP_NO_HEADER_INDEX))
	{
		throw HttpError("WinHttpQueryHeaders", GetLastError());
	}

	for (;;)
	{
		DWORD available = 0;
		if (!WinHttpQueryDataAvailable(request, &available)) throw HttpError("WinHttpQueryDataAvailable", GetLastError());
		if (available == 0) break;

		const size_t offset = response.body.size();
		response.body.resize(offset + available);

		DWORD read = 0;
		if (!WinHttpReadData(request, response.body.data() + offset, available, &read)) throw HttpError("WinHttpReadData", GetLastError());
		response.body.resize(offset + read);
	}

	return response;
}

void HttpClient::cancel()
{
	std::lock_guard lock(mutex_);
	cancelled_ = true;
	closeActiveRequestLocked();
}

void HttpClient::closeActiveRequestLocked()
{
	if (activeRequest_)
	{
		WinHttpCloseHandle(activeRequest_);
		activeRequest_ = nullptr;
	}
}
