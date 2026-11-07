#pragma once
#include <unordered_map>

#include "Globals.h"

class HotkeyManager
{
public:
	static std::unordered_map<int, int> hotkeysMap;

	/**
	@brief Set the contents of the hotkeyMap

	@param settings A reference to the SettingsStruct to receive hotkey data from
	*/
	static void setHotkeysMap(const SettingsStruct &settings);

	/**
	@brief If the keyCode is linked to a hotkey, post a message to MainWindow activate it.

	@param keyCode The virtual key to execute
	*/
	static void execute(int keyCode);
};
