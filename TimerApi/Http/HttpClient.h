#pragma once
#include <Windows.h>
#include <winhttp.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

// Timeouts for each phase of a request, in milliseconds. They only bound how long an
// unresponsive server takes to fail; exiting the app cancels requests immediately.
constexpr int HTTP_RESOLVE_TIMEOUT_MS = 5000;
constexpr int HTTP_CONNECT_TIMEOUT_MS = 5000;
constexpr int HTTP_SEND_TIMEOUT_MS = 5000;
constexpr int HTTP_RECEIVE_TIMEOUT_MS = 10000;

// HTTP methods
constexpr WCHAR HTTP_METHOD_GET[] = L"GET";

// A raw HTTP response. The body is left as the raw (UTF-8) bytes the server sent.
struct HttpResponse
{
	DWORD status = 0;
	std::string body;
};

// Thrown for any transport failure: a failed WinHTTP call, a timeout, or a cancellation.
// A non-2xx status code is not an HttpError; it is returned in HttpResponse.
class HttpError : public std::runtime_error
{
public:
	HttpError(const std::string& operation, DWORD code);

	/**
	 * @brief The Win32/WinHTTP error code (e.g. ERROR_WINHTTP_TIMEOUT).
	 */
	DWORD code() const { return code_; }

private:
	DWORD code_;
};

struct WinHttpHandleDeleter
{
	void operator()(HINTERNET handle) const { if (handle) WinHttpCloseHandle(handle); }
};

// Owning WinHTTP handle, closed automatically.
using WinHttpHandle = std::unique_ptr<void, WinHttpHandleDeleter>;

// A thin, synchronous WinHTTP client bound to a single host.
// Requests block, so call them from a worker thread, never the UI thread.
class HttpClient
{
private:
	bool https_;
	WinHttpHandle session_;
	WinHttpHandle connection_;

	// Guards the handle of the in-flight request, which cancel() closes from another thread.
	std::mutex mutex_;
	HINTERNET activeRequest_ = nullptr;
	bool cancelled_ = false;

public:
	/**
	 * @brief Create a client for a host. Doesn't touch the network. Uses HTTPS on port 443 unless told otherwise.
	 *
	 * @throws HttpError if WinHTTP can't be initialized.
	 */
	HttpClient(const std::wstring& host, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT, bool https = true);

	HttpClient(const HttpClient&) = delete;
	HttpClient& operator=(const HttpClient&) = delete;

	/**
	 * @brief Send a request and wait for the full response.
	 *
	 * @param method The HTTP method, e.g. HTTP_METHOD_GET.
	 * @param path The path and query, e.g. L"/api/status".
	 * @param body An optional JSON body.
	 *
	 * @throws HttpError on any transport failure or if the client was cancelled.
	 */
	HttpResponse request(const wchar_t* method, const std::wstring& path, const std::string& body = {});

	/**
	 * @brief Send a GET request and wait for the full response.
	 *
	 * @throws HttpError on any transport failure or if the client was cancelled.
	 */
	HttpResponse get(const std::wstring& path) { return request(HTTP_METHOD_GET, path); }

	/**
	 * @brief Abort the in-flight request, if any, and refuse all future requests.
	 * Thread-safe; the blocked request() call returns immediately with an HttpError.
	 */
	void cancel();

private:
	// Requires mutex_ to be held.
	void closeActiveRequestLocked();
};
