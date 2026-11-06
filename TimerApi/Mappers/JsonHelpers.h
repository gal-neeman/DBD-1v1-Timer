#pragma once
#include <json/json.h>
#include <optional>
#include <stdexcept>
#include <string>

// Thrown when a Timer API response can't be used: a bad status code, invalid JSON,
// or a missing or mistyped field.
class TimerApiError : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

/**
 * @brief Parse a response body into a JSON value.
 *
 * @throws TimerApiError if the body isn't valid JSON.
 */
Json::Value parseJson(const std::string& body);

/**
 * @brief Convert a UTF-8 string, as the API sends, to a wide string for the UI.
 */
std::wstring utf8ToWide(const std::string& utf8);

// Typed field readers. Each one throws TimerApiError if `obj` isn't an object,
// the key is missing, or the value has the wrong type.
bool readBool(const Json::Value& obj, const char* key);
int readInt(const Json::Value& obj, const char* key);
double readDouble(const Json::Value& obj, const char* key);
std::wstring readString(const Json::Value& obj, const char* key);
const Json::Value& readObject(const Json::Value& obj, const char* key);
const Json::Value& readArray(const Json::Value& obj, const char* key);

// Like readInt/readString, but a JSON null gives std::nullopt. The key must still exist.
std::optional<int> readOptionalInt(const Json::Value& obj, const char* key);
std::optional<std::wstring> readOptionalString(const Json::Value& obj, const char* key);
