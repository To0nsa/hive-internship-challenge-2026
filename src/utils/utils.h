#pragma once

#include <string>

static inline std::string formatMMSS(float seconds) {
    int  totalSeconds = static_cast<int>(seconds);
    int  minutes      = totalSeconds / 60;
    int  secs         = totalSeconds % 60;
    char buffer[14];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
    return std::string(buffer);
}