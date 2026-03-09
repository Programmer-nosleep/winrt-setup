#pragma once

#include <string>

struct WinRtSnapshot {
    std::string device_family;
    std::string device_version;
    std::string manufacturer;
    std::string product_name;
    std::string calendar_system;
    std::string time_zone;
    std::string last_updated;
};

WinRtSnapshot CaptureWinRtSnapshot();
