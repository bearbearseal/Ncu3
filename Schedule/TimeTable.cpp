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
    if (!eventMap.size())
    {
        eventMap[daySecond] = entry;
        printf("Write added.\n");
        return true;
    }
    auto upperBound = eventMap.upper_bound(daySecond);
    if (upperBound == eventMap.end())
    {
        eventMap[daySecond] = entry;
        printf("Write added.\n");
        return true;
    }
    if (upperBound->second.type != EventType::EndInterval)
    {
        eventMap[daySecond] = entry;
        printf("Write added.\n");
        return true;
    }
    printf("Cannot add write event.\n");
    return false;
}

void TimeTable::add_interval(const Value &value, uint32_t beginSecond, uint32_t endSecond)
{
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
    intervalMap[beginSecond] = intervalData;
    printf("Interval added.\n");
}

bool TimeTable::add_interval(const Value &value, const DayTime &begin, const DayTime &end)
{
    uint32_t beginSecond = ScheduleFunction::hour_minute_second_to_day_second(begin.hour, begin.minute, begin.second);
    uint32_t endSecond = ScheduleFunction::hour_minute_second_to_day_second(end.hour, end.minute, end.second);
    if (beginSecond >= endSecond)
    {
        printf("Cannot add interval 1.\n");
        return false;
    }
    if (!eventMap.size())
    {
        add_interval(value, beginSecond, endSecond);
        return true;
    }
    uint32_t mapLow = eventMap.begin()->first;
    uint32_t mapHigh = eventMap.rbegin()->first;
    if (beginSecond > mapHigh)
    {
        add_interval(value, beginSecond, endSecond);
        return true;
    }
    if (endSecond < mapLow)
    {
        add_interval(value, beginSecond, endSecond);
        return true;
    }
    if (beginSecond <= mapLow || endSecond >= mapHigh)
    {
        //Something in between
        printf("Cannot add interval 2.\n");
        return false;
    }
    auto lowerBound = eventMap.lower_bound(beginSecond);
    auto upperBound = eventMap.upper_bound(endSecond);
    if (lowerBound == upperBound)
    {
        if (upperBound->second.type != EventType::EndInterval)
        {
            add_interval(value, beginSecond, endSecond);
            return true;
        }
    }
    printf("Cannot add interval 3.\n");
    return false;
}

const TimeTable::IntervalData &TimeTable::get_interval_value(uint32_t daySecond) const
{
    auto lowerBound = intervalMap.find(daySecond);
    if (lowerBound == intervalMap.end())
    {
        return emptyInterval;
    }
    else if (lowerBound->second.endSecond < daySecond)
    {
        return emptyInterval;
    }
    return lowerBound->second;
}

TimeTable::EventType TimeTable::string_to_event_type(const std::string &eventType)
{
    string converted = eventType;
    transform(converted.begin(), converted.end(), converted.begin(), [](unsigned char c) { return std::tolower(c); });
    if (!converted.compare("start"))
    {
        return EventType::StartInterval;
    }
    else if (!converted.compare("end"))
    {
        return EventType::EndInterval;
    }
    else if (!converted.compare("write"))
    {
        return EventType::WriteValue;
    }
    return EventType::Invalid;
}

pair<uint32_t, const TimeTable::EventData &> TimeTable::get_the_event_after(uint32_t daySecond) const
{
    auto i = eventMap.upper_bound(daySecond);
    if (i == eventMap.end())
    {
        return emptyEventEntry;
    }
    return {i->first, i->second};
}

pair<uint32_t, const TimeTable::EventData &> TimeTable::get_the_event_at_or_after(uint32_t daySecond) const
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

pair<uint32_t, const TimeTable::EventData &> TimeTable::get_first_event() const
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
        printf("Cannot find event at dat second %u.\n", dataSecond);
        return emptyEvent;
    }
    return i->second;
}
