#pragma once
#include "TimerApi/Http/HttpClient.h"
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

// Runs HTTP jobs one at a time on a single long-lived thread, so requests never block the UI.
// An HttpError thrown by a job sets timerApiEnabled to false; the app keeps running.
class HttpWorker
{
public:
	using Job = std::function<void(HttpClient&)>;

private:
	std::wstring host_;
	INTERNET_PORT port_;
	bool https_;
	std::unique_ptr<HttpClient> client_;

	std::mutex mutex_;
	std::condition_variable_any condition_;
	std::deque<Job> jobs_;
	std::jthread thread_;

public:
	HttpWorker(std::wstring host, INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT, bool https = true);
	~HttpWorker();

	HttpWorker(const HttpWorker&) = delete;
	HttpWorker& operator=(const HttpWorker&) = delete;

	/**
	 * @brief Create the HTTP client and start the worker thread.
	 * If the client can't be created, timerApiEnabled is set to false and no thread starts.
	 */
	void start();

	/**
	 * @brief Queue a job to run on the worker thread. Ignored if the worker isn't running
	 * or the timer API is disabled.
	 *
	 * @param job Receives the client to send requests with. Runs on the worker thread.
	 */
	void enqueue(Job job);

	/**
	 * @brief Abort the in-flight request, drop queued jobs and join the thread. Returns immediately.
	 */
	void stop();

private:
	void run(std::stop_token stopToken);
};
