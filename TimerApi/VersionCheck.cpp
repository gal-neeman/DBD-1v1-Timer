#include "TimerApi/VersionCheck.h"
#include "globals.h"
#include <sstream>
#include <vector>

namespace
{
	// The numeric parts of a dotted version, e.g. L"1.10.0" -> {1, 10, 0}
	std::vector<int> versionParts(const std::wstring& version)
	{
		std::vector<int> parts;
		std::wistringstream stream(version);
		std::wstring part;
		while (std::getline(stream, part, L'.')) parts.push_back(_wtoi(part.c_str()));
		return parts;
	}

	// Compares part by part, so 1.10.0 is newer than 1.9.0
	bool isAppOlderThan(const std::wstring& version)
	{
		return versionParts(APP_VERSION) < versionParts(version);
	}
}

bool isUpdateRequired(const StatusDto& status)
{
	return isAppOlderThan(status.minSupportedVersion);
}

bool isUpdateAvailable(const StatusDto& status)
{
	return isAppOlderThan(status.latestVersion);
}
