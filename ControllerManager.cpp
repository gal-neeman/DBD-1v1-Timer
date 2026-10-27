#include "ControllerManager.h"

ControllerManager::ControllerManager()
{
	isRunning_ = false;
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
	if (isRunning_) return;

	isRunning_ = true;

	pollingThread_ = std::thread([this]()
	{
		while (isRunning_)
		{
			DWORD sleepDuration = poll() ? CONTROLLER_POLL_INTERVAL_MS : CONTROLLER_DISCONNECTED_POLL_INTERVAL_MS;
			Sleep(sleepDuration);
		}
	});
}

void ControllerManager::stop()
{
	isRunning_ = false;
	if (pollingThread_.joinable())
	{
		pollingThread_.join();
	}
}
