#include "TimerApi/Http/HttpWorker.h"
#include "globals.h"

HttpWorker::HttpWorker(std::wstring host, const INTERNET_PORT port, const bool https)
	: host_(std::move(host)), port_(port), https_(https)
{
}

HttpWorker::~HttpWorker()
{
	stop();
}

void HttpWorker::start()
{
	if (thread_.joinable()) return;

	try
	{
		client_ = std::make_unique<HttpClient>(host_, port_, https_);
	}
	catch (const HttpError&)
	{
		timerApiEnabled = false;
		return;
	}

	thread_ = std::jthread([this](const std::stop_token stopToken) { run(stopToken); });
}

bool HttpWorker::enqueue(Job job)
{
	if (!thread_.joinable()) return false;

	{
		std::lock_guard lock(mutex_);
		jobs_.push_back(std::move(job));
	}
	condition_.notify_one();
	return true;
}

void HttpWorker::stop()
{
	if (!thread_.joinable()) return;

	// Request the stop first so the job's HttpError from the cancellation isn't treated as a failure.
	thread_.request_stop();
	client_->cancel();
	thread_.join();
}

void HttpWorker::run(const std::stop_token stopToken)
{
	for (;;)
	{
		Job job;
		{
			std::unique_lock lock(mutex_);
			// Sleeps until a job arrives or a stop is requested, without polling.
			condition_.wait(lock, stopToken, [this] { return !jobs_.empty(); });
			if (stopToken.stop_requested()) return;

			job = std::move(jobs_.front());
			jobs_.pop_front();
		}

		try
		{
			job(*client_);
		}
		catch (...)
		{
			// An exception escaping a thread calls std::terminate, so never let one through.
			if (!stopToken.stop_requested()) timerApiEnabled = false;
		}
	}
}
