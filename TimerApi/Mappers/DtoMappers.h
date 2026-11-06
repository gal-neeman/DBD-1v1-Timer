#pragma once
#include "TimerApi/Dtos/PlayerDetailsDto.h"
#include "TimerApi/Dtos/StatusDto.h"
#include <string>

// Map raw response bodies to DTOs. Each throws TimerApiError if the body doesn't match the DTO.

StatusDto mapStatus(const std::string& body);

PlayerDetailsDto mapPlayerDetails(const std::string& body);
