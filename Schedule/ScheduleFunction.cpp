#include "ScheduleFunction.h"
#include <stdio.h>

using namespace std;

ScheduleFunction::HourMinuteSecond ScheduleFunction::chrono_time_to_hms(const std::chrono::time_point<std::chrono::system_clock> &theTime)
{
    time_t totalSec = chrono::system_clock::to_time_t(theTime);
    tm theTm;
    localtime_r(&totalSec, &theTm);
    return {uint8_t(theTm.tm_hour), uint8_t(theTm.tm_min), uint8_t(theTm.tm_sec)};
}

uint32_t ScheduleFunction::chrono_time_to_day_second(const std::chrono::time_point<std::chrono::system_clock> &theTime)
{
    time_t totalSec = chrono::system_clock::to_time_t(theTime);
    tm theTm;
    localtime_r(&totalSec, &theTm);
    uint32_t retVal = theTm.tm_hour * 3600 + theTm.tm_min * 60 + theTm.tm_sec;
    return retVal;
}

uint32_t ScheduleFunction::hour_minute_second_to_day_second(uint8_t hour, uint8_t minute, uint8_t second)
{
    uint32_t retVal = hour;
    retVal *= 60;
    retVal += minute;
    retVal *= 60;
    retVal += second;
    return retVal;
}

ScheduleFunction::HourMinuteSecond ScheduleFunction::day_time_to_hms(uint32_t theTime)
{
    time_t secNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    time_t secToday = secNow%(24*3600);
    time_t theMoment = secNow - secToday + theTime;
    tm localTime;
    localtime_r(&theMoment, &localTime);
    return {uint8_t(localTime.tm_hour), uint8_t(localTime.tm_min), uint8_t(localTime.tm_sec)};
}

time_t ScheduleFunction::today_hms_to_local_time_t(uint8_t hour, uint8_t minute, uint8_t second)
{
    time_t secNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm localTime;
    localtime_r(&secNow, &localTime);
    localTime.tm_hour = hour;
    localTime.tm_min = minute;
    localTime.tm_sec = second;
    return mktime(&localTime);
}

time_t ScheduleFunction::today_second_to_local_time_t(uint32_t daySecond)
{
    time_t secNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm localTime;
    localtime_r(&secNow, &localTime);
    localTime.tm_hour = (daySecond/(24*3600))%24;
    localTime.tm_min = (daySecond/60)%60;
    localTime.tm_sec = daySecond%60;
    return mktime(&localTime);
}

uint32_t ScheduleFunction::get_day_second_of(time_t theTime)
{
    tm localTime;
    localtime_r(&theTime, &localTime);
    uint32_t retVal = localTime.tm_hour;
    retVal *= 60;
    retVal += localTime.tm_min;
    retVal *= 60;
    retVal += localTime.tm_sec;
    return retVal;
}
