#ifndef _ScheduleFunction_H_
#define _ScheduleFunction_H_
#include <stdint.h>
#include <chrono>

namespace ScheduleFunction
{
    struct HourMinuteSecond
    {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };

    HourMinuteSecond chrono_time_to_hms(const std::chrono::time_point<std::chrono::system_clock> &theTime);

    uint32_t chrono_time_to_day_second(const std::chrono::time_point<std::chrono::system_clock> &theTime);

    uint32_t hour_minute_second_to_day_second(uint8_t hour, uint8_t minute, uint8_t second);

    HourMinuteSecond day_time_to_hms(uint32_t theTime);

    time_t today_hms_to_local_time_t(uint8_t hour, uint8_t minute, uint8_t second);

    time_t today_second_to_local_time_t(uint32_t daySecond);

    uint32_t get_day_second_of(time_t theTime);
}

#endif