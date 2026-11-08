#pragma once
#include "TimerApi/Dtos/StatusDto.h"

// Compare APP_VERSION with the versions in a status response. A blank version never counts as newer.

// True when APP_VERSION is older than minSupportedVersion, so the API must not be used.
bool isUpdateRequired(const StatusDto& status);

// True when APP_VERSION is older than latestVersion.
bool isUpdateAvailable(const StatusDto& status);
