#include "TimeTable.h"
#include "ScheduleFunction.h"
#include <algorithm>

using namespace std;

//pair<uint32_t, Value> emptyEvent = {0, Value()};
const std::pair<uint32_t, TimeTable::EventData> emptyEventEntry = {0, {TimeTable::EventType::Invalid, Value()}};
TimeTable::IntervalData emptyInterval = {0, Value()};
TimeTable::EventData emptyEvent;

TimeTable::TimeTable()
{
}

TimeTable::~TimeTable()
{
}

bool TimeTable::add_write_event(const Value &value, const DayTime &dayTime)
{
    EventData entry;
    entry.type = EventType::WriteValue;
    entry.value = value;
    uint32_t daySecond = ScheduleFunction::hour_minute_second_to_day_second(dayTime.hour, dayTime.minute, dayTime.second);
    auto lower = eventMap.lower_bound(daySecond);
    if (lower == eventMap.end())
    {
        eventMap[daySecond] = entry;
        return true;
    }
    if (lower->second.type != EventType::StartInterval)
    {
        eventMap[daySecond] = entry;
        return true;
    }
    return false;
}

bool TimeTable::add_interval(const Value &value, const DayTime &begin, const DayTime &end)
{
    uint32_t beginSecond = ScheduleFunction::hour_minute_second_to_day_second(begin.hour, begin.minute, begin.second);
    uint32_t endSecond = ScheduleFunction::hour_minute_second_to_day_second(end.hour, end.minute, end.second);
    if (beginSecond >= endSecond)
    {
        return false;
    }
    //Make sure the beginTime is not > previous
    auto lowerBound = eventMap.lower_bound(beginSecond);
    auto upperBound = eventMap.upper_bound(endSecond);
    if (lowerBound->first == beginSecond)
    {
        return false;
    }
    if (lowerBound == eventMap.end())
    {
        if (upperBound == eventMap.end() || upperBound == eventMap.begin())
        {
            //can, nothing in between
            goto can;
        }
    }
    else
    {
        auto lowerNext = lowerBound;
        ++lowerNext;
        if (upperBound == lowerNext)
        {
            //can, nothing in between
            goto can;
        }
    }
    return false;
can:
    EventData beginner;
    beginner.type = EventType::StartInterval;
    beginner.value = value;
    eventMap[beginSecond] = beginner;
    EventData ender;
    ender.type = EventType::EndInterval;
    eventMap[endSecond] = ender;
    IntervalData intervalData;
    intervalData.endSecond = endSecond;
    intervalData.value = value;
    return true;
}

const TimeTable::IntervalData &TimeTable::get_interval_value(uint32_t daySecond) const
{
    printf("Getting interval value.\n");
    auto lowerBound = intervalMap.find(daySecond);
    if (lowerBound == intervalMap.end())
    {
        printf("Empty interval 1.\n");
        return emptyInterval;
    }
    else if (lowerBound->second.endSecond < daySecond)
    {
        printf("EmptyInterval 2.\n");
        return emptyInterval;
    }
    printf("Returning valid result.\n");
    return lowerBound->second;
}

TimeTable::EventType TimeTable::string_to_event_type(const std::string& eventType)
{
    string converted = eventType;
    transform(converted.begin(), converted.end(), converted.begin(), [](unsigned char c){ return std::tolower(c); });
    if(!converted.compare("start"))
    {
        return EventType::StartInterval;
    }
    else if(!eventType.compare("end"))
    {
        return EventType::EndInterval;
    }
    else if(!eventType.compare("write"))
    {
        return EventType::WriteValue;
    }
    return EventType::Invalid;
}

pair<uint32_t, const TimeTable::EventData&> TimeTable::get_the_event_after(uint32_t daySecond) const
{
    printf("In get event.\n");
    auto i = eventMap.upper_bound(daySecond);
    printf("Got upper bound.\n");
    if (i == eventMap.end())
    {
        printf("Returning empty event.\n");
        return emptyEventEntry;
    }
    printf("Returning event after: %u\n", daySecond);
    return {i->first, i->second};
}

pair<uint32_t, const TimeTable::EventData&> TimeTable::get_the_event_at_or_after(uint32_t daySecond) const
{
    auto i = eventMap.find(daySecond);
    if (i == eventMap.end())
    {
        i = eventMap.upper_bound(daySecond);
        if (i == eventMap.end())
        {
            return emptyEventEntry;
        }
    }
    return {i->first, i->second};
}

pair<uint32_t, const TimeTable::EventData&> TimeTable::get_first_event() const
{
    if (eventMap.size())
    {
        return {eventMap.begin()->first, eventMap.begin()->second};
    }
    return emptyEventEntry;
}

const TimeTable::EventData &TimeTable::get_event_data(uint32_t dataSecond) const
{
    auto i = eventMap.find(dataSecond);
    if (i == eventMap.end())
    {
        return emptyEvent;
    }
    return i->second;
}
