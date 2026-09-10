#include <fstream>
#include "SettingsUtils.h"

#include "HotkeyManager.h"
#include "dist/json/json.h"

using namespace std;

Json::Value settingsToJson(const SettingsStruct& settings)
{
	Json::Value settingsJson;

	settingsJson["start"] = settings.startKey;
	settingsJson["timer1"] = settings.timer1Key;
	settingsJson["timer2"] = settings.timer2Key;
	settingsJson["startNoReset"] = settings.startNoResetKey;

	settingsJson["conStart"] = settings.conStartKey;
	settingsJson["conTimer1"] = settings.conTimer1Key;
	settingsJson["conTimer2"] = settings.conTimer2Key;
	settingsJson["conStartNoReset"] = settings.conStartNoResetKey;

	settingsJson["optionTransparent"] = settings.optionTransparent;
	settingsJson["optionStartOnChange"] = settings.optionStartOnChange;
	settingsJson["optionLastSecondsTime"] = settings.optionLastSecondsTime;

	settingsJson["colors"]["timer"] = settings.colors.timerColor;
	settingsJson["colors"]["selected timer"] = settings.colors.selectedTimerColor;
	settingsJson["colors"]["last seconds"] = settings.colors.lastSecondsColor;
	settingsJson["colors"]["background"] = settings.colors.backgroundColor;

	return settingsJson;
}

void settingsFromJson(const Json::Value& json, SettingsStruct& settings)
{
	// hotkeys
	if (json["start"].isInt() && json["timer1"].isInt() && json["timer2"].isInt() &&
		json["conStart"].isInt() && json["conTimer1"].isInt() && json["conTimer2"].isInt()
		&& json["startNoReset"].isInt() && json["conStartNoReset"].isInt())
	{
		settings.startKey = json["start"].asInt();
		settings.timer1Key = json["timer1"].asInt();
		settings.timer2Key = json["timer2"].asInt();
		settings.startNoResetKey = json["startNoReset"].asInt();

		settings.conStartKey = json["conStart"].asInt();
		settings.conTimer1Key = json["conTimer1"].asInt();
		settings.conTimer2Key = json["conTimer2"].asInt();
		settings.conStartNoResetKey = json["conStartNoReset"].asInt();
	}

	// options
	if (json["optionTransparent"].isBool() && json["optionStartOnChange"].isBool()) {
		settings.optionTransparent = json["optionTransparent"].asBool();
		settings.optionStartOnChange = json["optionStartOnChange"].asBool();
		settings.optionLastSecondsTime = json["optionLastSecondsTime"].asInt();
	}

	settings.optionClickThrough = false;

	// colors
	Json::Value colors = json["colors"];
	if (colors["timer"].isInt() && colors["selected timer"].isInt()
		&& colors["last seconds"].isInt() && colors["background"].isInt())
	{
		settings.colors.timerColor = colors["timer"].asInt();
		settings.colors.selectedTimerColor = colors["selected timer"].asInt();
		settings.colors.lastSecondsColor = colors["last seconds"].asInt();
		settings.colors.backgroundColor = colors["background"].asInt();

		if (settings.colors.timerColor > 24 || settings.colors.selectedTimerColor > 24 ||
			settings.colors.lastSecondsColor > 24 || settings.colors.backgroundColor > 24)
		{
			settings.colors.timerColor = 9;
			settings.colors.selectedTimerColor = 6;
			settings.colors.lastSecondsColor = 1;
			settings.colors.backgroundColor = 20;
		}
	}
}

SettingsStruct getSafeSettingsStruct()
{
	ifstream file(SETTINGS_FILE_NAME);
	Json::Value actualJson;
	Json::Reader reader;
	SettingsStruct settings;

	reader.parse(file, actualJson);

	settingsFromJson(actualJson, settings);

	return settings;
}

void setSettingsStruct(const SettingsStruct& settings)
{
	const Json::Value settingsJson = settingsToJson(settings);

	// Write to file
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";

	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	std::ofstream outputFileStream(SETTINGS_FILE_NAME);
	writer->write(settingsJson, &outputFileStream);
}

void createSettingsFile()
{
	const SettingsStruct defaultSettings;
	const Json::Value settingsJson = settingsToJson(defaultSettings);

	// Write to file
	Json::StreamWriterBuilder builder;

	builder["commentStyle"] = "None";
	builder["indentation"] = "   ";
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	std::ofstream outputFileStream(SETTINGS_FILE_NAME);
	writer->write(settingsJson, &outputFileStream);
}

bool settingsFileExists() {
	const string name = SETTINGS_FILE_NAME;

	const ifstream f(name.c_str());
	return f.good();
}

void applySettings(const SettingsStruct& settings) {
	setSettingsStruct(settings); // write to json file (settings.json)
	appSettings = settings; // save global settings variable
	HotkeyManager::setHotkeysMap(appSettings); // initialize hotkeys map

	if (hwndMainWindow != nullptr) {
		// transparency
		if (appSettings.optionTransparent) { // add transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 0, 0, LWA_COLORKEY);
		}
		else { // remove transparent effect
			SetLayeredWindowAttributes(hwndMainWindow, 0, 255, LWA_ALPHA);
		}

		// click through
		if (appSettings.optionClickThrough) { // make click through
			// Get the current window style
			LONG style = GetWindowLong(hwndMainWindow, GWL_EXSTYLE);

			// Add the new style to the current styles
			style |= WS_EX_TRANSPARENT;

			// Set the new style
			SetWindowLong(hwndMainWindow, GWL_EXSTYLE, style);
		}
	}
}
