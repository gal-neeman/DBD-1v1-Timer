#pragma once

#include "Globals.h"
#include <json/json.h>

/**
@brief Safely loads the saved settings from the settings.json file. Should only be called once, on startup.
		A missing or malformed file leaves the given struct's existing values in place.

@param settings The settings struct to load into.
*/
void loadSettingsFromFile(SettingsStruct& settings);

/**
@brief Save settings to local variable and json file (also apply temporary settings).

@param settings The settings to be applied.
*/
void applySettings(const SettingsStruct& settings);

/**
@brief Builds the canonical JSON representation of a SettingsStruct.

@param settings The settings struct to convert.

@return The Json::Value representing the given settings.
*/
Json::Value settingsToJson(const SettingsStruct& settings);

/**
@brief Fills a SettingsStruct's fields from JSON, validating each field group and
		leaving the existing value(s) in place if the group is missing or malformed.

@param json The Json::Value to read from.

@param settings The settings struct to fill (its existing values act as fallback).
*/
void settingsFromJson(const Json::Value& json, SettingsStruct& settings);
