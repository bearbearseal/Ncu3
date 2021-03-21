/****************************************************
 * Bug: Interval should be inclusive of the end
 ****************************************************/

#ifndef _TimeTable_H_
#define _TimeTable_H_
#include <memory>
#include <map>
#include <chrono>
#include "ScheduleRule.h"
#include "../VariableTree/VariableTree.h"
#include "../../MyLib/Basic/Variable.h"
#include "../../MyLib/ITC/ITC.h"

//Has information of what day what schedule to follow
class TimeTable
{
public:
    struct IntervalData
    {
        uint32_t endSecond;
        Value value;
    };
    struct DayTime
    {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };
    enum class EventType
    {
        StartInterval,
        EndInterval,
        WriteValue,
        Invalid
    };
    static bool is_event_type(int eventType) { return (eventType >= int(EventType::StartInterval) && eventType <= int(EventType::Invalid)); }
    static bool is_valid_event_type(int eventType) { return (eventType >= int(EventType::StartInterval) && eventType <= int(EventType::WriteValue)); }
    static EventType string_to_event_type(const std::string& eventType);
    struct EventData
    {
        EventType type;
        Value value;
    };
    TimeTable();
    virtual ~TimeTable();

    bool add_write_event(const Value &value, const DayTime &eventTime);
    //Bug, should be inclusive of begin and end
    bool add_interval(const Value &value, const DayTime &begin, const DayTime &end);

    //Return endSecond max value if no interval found
    const IntervalData &get_interval_value(uint32_t daySecond) const;
    std::pair<uint32_t, const EventData &> get_the_event_after(uint32_t daySecond) const;
    std::pair<uint32_t, const EventData &> get_the_event_at_or_after(uint32_t daySecond) const;
    std::pair<uint32_t, const EventData &> get_first_event() const;
    const EventData &get_event_data(uint32_t dataSecond) const;
    //excluding same value, strictly > and <, not >= and <=
    //std::pair<const std::pair<uint32_t, EventData>&, const std::pair<uint32_t, EventData>&> get_bound_events(uint32_t daySecond) const;

private:
    std::map<uint32_t, EventData> eventMap;
    std::map<uint32_t, IntervalData> intervalMap;

    void add_interval(const Value& value, uint32_t beginSecond, uint32_t endSecond);
};

#endif