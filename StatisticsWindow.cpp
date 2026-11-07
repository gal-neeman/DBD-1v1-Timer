#include <Windows.h>
#include "Globals.h"
#include "StatisticsWindow.h"
#include "Program.h"
#include <exception>
#include <winhttp.h>

void StatisticsWindow::setStatus(const std::wstring& message)
{
	statusMessage_ = message;
	setState(derivedState());
}

void StatisticsWindow::setModel(std::optional<StatsWindowModel> model)
{
	model_ = std::move(model);
	setState(derivedState());
}

void StatisticsWindow::setError(const TimerApiRequest request, const DWORD code)
{
	if (request == TimerApiRequest::Status)
	{
		statusMessage_.clear(); // the request failed, so there's no message from the API
		setState(StatsState::Disabled);
	}
	else
	{
		setState(code == HTTP_STATUS_NOT_FOUND ? StatsState::PlayerNotFound : StatsState::Error);
	}
}

StatsState StatisticsWindow::derivedState() const
{
	if (!timerApiEnabled) return StatsState::Disabled;
	if (model_) return StatsState::Loaded;
	if (pGlobalTimerWindow && !pGlobalTimerWindow->activePlayerId().empty()) return StatsState::Loading;
	return StatsState::NeedsId;
}

void StatisticsWindow::setState(const StatsState state)
{
	state_ = state;
	if (hwnd_) InvalidateRect(hwnd_, nullptr, TRUE);
}

LRESULT StatisticsWindow::handleMessage(const UINT wMsg, const WPARAM wParam, const LPARAM lParam)
{
	try
	{
		switch (wMsg)
		{
		case WM_DESTROY:
			hwnd_ = nullptr;
			return 0;
		default:
			break;
		}
	}
	catch (const std::exception &e)
	{
		exitApp();
	}
	return DefWindowProc(window(), wMsg, wParam, lParam);
}
