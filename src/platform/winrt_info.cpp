#include "platform/winrt_info.h"

#include <charconv>
#include <ctime>
#include <string>

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Security.ExchangeActiveSyncProvisioning.h>
#include <winrt/Windows.System.Profile.h>

namespace {

std::string ToUtf8(winrt::hstring const& value)
{
    return winrt::to_string(value);
}

std::string SafeText(std::string value, std::string fallback = "Unavailable")
{
    return value.empty() ? fallback : value;
}

std::string FormatDeviceFamilyVersion(std::string const& raw)
{
    std::uint64_t value = 0;
    auto begin = raw.data();
    auto end = raw.data() + raw.size();
    auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc()) {
        return SafeText(raw);
    }

    return fmt::format(
        "{}.{}.{}.{}",
        (value >> 48) & 0xFFFFu,
        (value >> 32) & 0xFFFFu,
        (value >> 16) & 0xFFFFu,
        value & 0xFFFFu
    );
}

std::string CurrentTimestamp()
{
    const auto now = std::time(nullptr);
    std::tm local_tm{};
    localtime_s(&local_tm, &now);
    return fmt::format("{:%Y-%m-%d %H:%M:%S}", local_tm);
}

} // namespace

WinRtSnapshot CaptureWinRtSnapshot()
{
    using winrt::Windows::Globalization::Calendar;
    using winrt::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation;
    using winrt::Windows::System::Profile::AnalyticsInfo;

    Calendar calendar;
    EasClientDeviceInformation device_info;
    auto version_info = AnalyticsInfo::VersionInfo();

    return WinRtSnapshot{
        .device_family = SafeText(ToUtf8(version_info.DeviceFamily())),
        .device_version = FormatDeviceFamilyVersion(ToUtf8(version_info.DeviceFamilyVersion())),
        .manufacturer = SafeText(ToUtf8(device_info.SystemManufacturer())),
        .product_name = SafeText(ToUtf8(device_info.SystemProductName())),
        .calendar_system = SafeText(ToUtf8(calendar.GetCalendarSystem())),
        .time_zone = SafeText(ToUtf8(calendar.GetTimeZone())),
        .last_updated = CurrentTimestamp(),
    };
}
