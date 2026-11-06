#include <fstream>
#include <algorithm>
#include "SettingsUtils.h"

#include "HotkeyManager.h"
#include "Program.h"
#include "ResourceUtils.h"
#include <json/json.h>

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

	// The ID is digits only, so narrowing it to a plain string is lossless
	string discordId;
	for (const wchar_t c : settings.league.discordId) discordId += static_cast<char>(c);
	settingsJson["league"]["discordId"] = discordId;
	settingsJson["league"]["statsOverlayEnabled"] = settings.league.statsOverlayEnabled;
	settingsJson["league"]["showNamesAndScore"] = settings.league.showNamesAndScore;

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
	if (
		json["optionTransparent"].isBool() && 
		json["optionStartOnChange"].isBool()) {
		settings.optionTransparent = json["optionTransparent"].asBool();
		settings.optionStartOnChange = json["optionStartOnChange"].asBool();
	}

	// numeric options
	if (json["optionLastSecondsTime"].isInt()) {
		settings.optionLastSecondsTime = json["optionLastSecondsTime"].asInt();

		if (settings.optionLastSecondsTime < 0 || settings.optionLastSecondsTime > 99) {
			settings.optionLastSecondsTime = 20;
		}
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

		if (settings.colors.timerColor >= PALETTE_SIZE || settings.colors.timerColor < 0 ||
			settings.colors.selectedTimerColor >= PALETTE_SIZE || settings.colors.selectedTimerColor < 0 ||
			settings.colors.lastSecondsColor >= PALETTE_SIZE || settings.colors.lastSecondsColor < 0 ||
			settings.colors.backgroundColor >= PALETTE_SIZE || settings.colors.backgroundColor < 0
			)
		{
			settings.colors.timerColor = 9;
			settings.colors.selectedTimerColor = 6;
			settings.colors.lastSecondsColor = 1;
			settings.colors.backgroundColor = 20;
		}
	}

	// league: an empty ID is valid, a malformed one leaves the whole group at its defaults
	const Json::Value& league = json["league"];
	if (league.isObject() && league["discordId"].isString() &&
		league["statsOverlayEnabled"].isBool() && league["showNamesAndScore"].isBool())
	{
		const string id = league["discordId"].asString();
		const wstring wideId(id.begin(), id.end());
		if (wideId.empty() || isValidDiscordId(wideId))
		{
			settings.league.discordId = wideId;
			settings.league.statsOverlayEnabled = !wideId.empty() && league["statsOverlayEnabled"].asBool();
			settings.league.showNamesAndScore = league["showNamesAndScore"].asBool();
		}
	}
}

bool isValidDiscordId(const wstring& id)
{
	return id.size() >= 17 && id.size() <= 20 &&
		all_of(id.begin(), id.end(), [](const wchar_t c) { return c >= L'0' && c <= L'9'; });
}

void loadSettingsFromFile(SettingsStruct& settings)
{
	ifstream file(SETTINGS_FILE_NAME);
	Json::Value actualJson;
	Json::Reader reader;

	reader.parse(file, actualJson);

	settingsFromJson(actualJson, settings);
}

static void saveSettingsToFile(const SettingsStruct& settings)
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

void applySettings(const SettingsStruct& settings) {
	saveSettingsToFile(settings); // write to json file (settings.json)
	appSettings = settings; // save global settings variable
	HotkeyManager::setHotkeysMap(appSettings); // initialize hotkeys map

	if (pGlobalTimerWindow != nullptr) {
		pGlobalTimerWindow->applyWindowSettings(appSettings);
	}
}
