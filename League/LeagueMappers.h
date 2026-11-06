#pragma once
#include "League/Models/OverlayStats.h"
#include "League/Models/StatsWindowModel.h"
#include "TimerApi/Dtos/PlayerDetailsDto.h"

// Map a player's API details to what each view shows. These never throw: they run on the UI thread.

OverlayStats toOverlayStats(const PlayerDetailsDto& dto);

StatsWindowModel toStatsWindowModel(const PlayerDetailsDto& dto);
