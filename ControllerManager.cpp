#include "ControllerManager.h"

ControllerManager::ControllerManager()
{
	stopEvent_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

ControllerManager::~ControllerManager()
{
	stop();
	CloseHandle(stopEvent_);
}

void ControllerManager::setInputCallback(const std::function<void(WORD)>& callback)
{
	inputCallback_ = callback;
}

bool ControllerManager::poll()
{
	const bool connected = XInputGetState(0, &state_) == ERROR_SUCCESS;

	if (connected)
	{
		const bool leftTriggerChanged = state_.Gamepad.bLeftTrigger / 255 != previousState_.Gamepad.bLeftTrigger / 255;
		const bool rightTriggerChanged = state_.Gamepad.bRightTrigger / 255 != previousState_.Gamepad.bRightTrigger / 255;

		const bool leftTriggerDown = leftTriggerChanged && state_.Gamepad.bLeftTrigger / 255 == 1;
		const bool rightTriggerDown = rightTriggerChanged && state_.Gamepad.bRightTrigger / 255 == 1;

		const bool buttonsChanged = state_.Gamepad.wButtons != 0 && state_.Gamepad.wButtons != previousState_.Gamepad.wButtons;

		if (buttonsChanged)
		{
			WORD buttons = buttonsMap[state_.Gamepad.wButtons];

 			inputCallback_(buttons);
		}
		else if (leftTriggerDown)
		{
			inputCallback_(ControllerButtons::LeftTrigger);
		}
		else if (rightTriggerDown)
		{
			inputCallback_(ControllerButtons::RightTrigger);
		}

		previousState_ = state_;
	}

	return connected;
}

void ControllerManager::start()
{
	if (pollingThread_.joinable()) return;

	ResetEvent(stopEvent_);

	pollingThread_ = std::thread([this]()
	{
		DWORD waitDuration = 0;
		while (WaitForSingleObject(stopEvent_, waitDuration) == WAIT_TIMEOUT)
		{
			waitDuration = poll() ? CONTROLLER_POLL_INTERVAL_MS : CONTROLLER_DISCONNECTED_POLL_INTERVAL_MS;
		}
	});
}

void ControllerManager::stop()
{
	SetEvent(stopEvent_);
	if (pollingThread_.joinable())
	{
		pollingThread_.join();
	}
}
