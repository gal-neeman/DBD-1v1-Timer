#pragma once

#include "Globals.h"
#include "dist/json/json.h"

/**
@brief Safely retrieves the SettingsStruct from the settings.json file.
		Handles errors in the settings.json file and resets it if need be.

@return The SettingsStruct representing the saved settings from the settings.json file.
*/
SettingsStruct getSafeSettingsStruct();

/**
@brief Writes the settings from the given SettingsStruct to the settings.json file.

@param settings The settings struct to save to the file.
*/
void setSettingsStruct(const SettingsStruct& settings);

/**
@brief Initial creation of the settings.json file.
*/
void createSettingsFile();

/**
@return Wether settings.json exists or not.
*/
bool settingsFileExists();

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
