#pragma once

#include "BaseWindow.h"
#include <functional>
#include <map>
#include <thread>
#include <Xinput.h>

constexpr DWORD CONTROLLER_POLL_INTERVAL_MS = 20;
constexpr DWORD CONTROLLER_DISCONNECTED_POLL_INTERVAL_MS = 5000;

enum ControllerButtons : USHORT
{
	A = CONTROLLER_A,
	B = CONTROLLER_B,
	X = CONTROLLER_X,
	Y = CONTROLLER_Y,
	UpArrow = CONTROLLER_UP,
	DownArrow = CONTROLLER_DOWN,
	RightArrow = CONTROLLER_RIGHT,
	LeftArrow = CONTROLLER_LEFT,
	Start = CONTROLLER_START,
	Back = CONTROLLER_BACK,
	LeftThumb = CONTROLLER_LEFT_THUMB,
	RightThumb = CONTROLLER_RIGHT_THUMB,
	LeftShoulder = CONTROLLER_LEFT_SHOULDER,
	RightShoulder = CONTROLLER_RIGHT_SHOULDER,
	LeftTrigger = CONTROLLER_LEFT_TRIGGER,
	RightTrigger = CONTROLLER_RIGHT_TRIGGER
};

class ControllerManager
{
public:
	std::map<USHORT, USHORT> buttonsMap = {
		{XINPUT_GAMEPAD_DPAD_UP, CONTROLLER_UP},
		{XINPUT_GAMEPAD_DPAD_DOWN, CONTROLLER_DOWN},
		{XINPUT_GAMEPAD_DPAD_LEFT, CONTROLLER_LEFT},
		{XINPUT_GAMEPAD_DPAD_RIGHT, CONTROLLER_RIGHT},
		{XINPUT_GAMEPAD_START, CONTROLLER_START},
		{PS_CONTROLLER_START, CONTROLLER_START},
		{XINPUT_GAMEPAD_BACK, CONTROLLER_BACK},
		{PS_CONTROLLER_BACK, CONTROLLER_BACK},
		{XINPUT_GAMEPAD_LEFT_THUMB, CONTROLLER_LEFT_THUMB},
		{PS_CONTROLLER_L3, CONTROLLER_LEFT_THUMB},
		{XINPUT_GAMEPAD_RIGHT_THUMB, CONTROLLER_RIGHT_THUMB},
		{PS_CONTROLLER_R3, CONTROLLER_RIGHT_THUMB},
		{XINPUT_GAMEPAD_LEFT_SHOULDER, CONTROLLER_LEFT_SHOULDER},
		{PS_CONTROLLER_LB, CONTROLLER_LEFT_SHOULDER},
		{XINPUT_GAMEPAD_RIGHT_SHOULDER, CONTROLLER_RIGHT_SHOULDER},
		{PS_CONTROLLER_RB, CONTROLLER_RIGHT_SHOULDER},
		{XINPUT_GAMEPAD_A, CONTROLLER_A},
		{PS_CONTROLLER_A, CONTROLLER_A},
		{XINPUT_GAMEPAD_B, CONTROLLER_B},
		{PS_CONTROLLER_B, CONTROLLER_B},
		{XINPUT_GAMEPAD_X, CONTROLLER_X},
		{PS_CONTROLLER_X, CONTROLLER_X},
		{XINPUT_GAMEPAD_Y, CONTROLLER_Y},
		{PS_CONTROLLER_Y, CONTROLLER_Y},
	};

private:
	XINPUT_STATE state_ = {};
	XINPUT_STATE previousState_ = {};
	std::function<void(WORD)> inputCallback_;
	HANDLE stopEvent_;
	std::thread pollingThread_;

public:
	ControllerManager();
	~ControllerManager();

	/**
	 * @brief Set a callback method to be called for input events.
	 * 
	 * @param callback The method to be called.
	 */
	void setInputCallback(const std::function<void(WORD)>& callback);

	/**
	 * @brief Start listening for controller input.
	 */
	void start();

	/**
	 * @brief Stop listening for controller input.
	 */
	void stop();

	void initializeButtonsMap();

private:
	/**
	 * @brief Retreive controller input data.
	 *
	 * @return Whether a controller is connected.
	 */
	bool poll();
};
