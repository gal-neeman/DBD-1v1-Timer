#include "TimerApi/Mappers/JsonHelpers.h"
#include <Windows.h>
#include <memory>

namespace
{
	// Returns the field, or throws if `obj` isn't an object or doesn't have the key.
	const Json::Value& field(const Json::Value& obj, const char* key)
	{
		if (!obj.isObject() || !obj.isMember(key)) throw TimerApiError(std::string("missing field: ") + key);
		return obj[key];
	}

	[[noreturn]] void wrongType(const char* key, const char* expected)
	{
		throw TimerApiError(std::string("field '") + key + "' is not " + expected);
	}
}

Json::Value parseJson(const std::string& body)
{
	Json::CharReaderBuilder builder;
	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

	Json::Value root;
	std::string errors;
	if (!reader->parse(body.data(), body.data() + body.size(), &root, &errors))
	{
		throw TimerApiError("invalid JSON: " + errors);
	}
	return root;
}

std::wstring utf8ToWide(const std::string& utf8)
{
	if (utf8.empty()) return {};

	const int size = static_cast<int>(utf8.size());
	const int length = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), size, nullptr, 0);
	std::wstring wide(length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, utf8.data(), size, wide.data(), length);
	return wide;
}

bool readBool(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isBool()) wrongType(key, "a bool");
	return value.asBool();
}

int readInt(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isInt()) wrongType(key, "an int");
	return value.asInt();
}

double readDouble(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isNumeric()) wrongType(key, "a number");
	return value.asDouble();
}

std::wstring readString(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isString()) wrongType(key, "a string");
	return utf8ToWide(value.asString());
}

const Json::Value& readObject(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isObject()) wrongType(key, "an object");
	return value;
}

const Json::Value& readArray(const Json::Value& obj, const char* key)
{
	const Json::Value& value = field(obj, key);
	if (!value.isArray()) wrongType(key, "an array");
	return value;
}

std::optional<int> readOptionalInt(const Json::Value& obj, const char* key)
{
	if (field(obj, key).isNull()) return std::nullopt;
	return readInt(obj, key);
}

std::optional<std::wstring> readOptionalString(const Json::Value& obj, const char* key)
{
	if (field(obj, key).isNull()) return std::nullopt;
	return readString(obj, key);
}
