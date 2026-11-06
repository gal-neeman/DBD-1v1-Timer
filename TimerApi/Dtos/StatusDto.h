#pragma once
#include <string>

// Response of GET /api/v1/status.
struct StatusDto
{
	bool statusEnabled = false;
	int fastPollSec = 0;
	int slowPollSec = 0;
	int fastPollCapSec = 0;
	std::wstring latestVersion;
	std::wstring minSupportedVersion;
	std::wstring statusMessage;
};
