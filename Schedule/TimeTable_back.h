/****************************************************
 * Interval is inclusive of the seconds at begin and end.
 * Time table only deal with hour, minutes and seconds, no date involved.
 * Events should be added during init stage and should only be read after that.
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
#include "../Global/GlobalEnum.h"

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
    struct ActionData
    {
        uint32_t daySecond;
        GlobalEnum::ScheduleAction action;
        Value value;
    };
    /*
    enum class EventType
    {
        StartInterval,
        EndInterval,
        WriteValue,
        Invalid
    };
    */
    //static bool is_event_type(int eventType) { return (eventType >= int(EventType::StartInterval) && eventType <= int(EventType::Invalid)); }
    //static bool is_valid_event_type(int eventType) { return (eventType >= int(EventType::StartInterval) && eventType <= int(EventType::WriteValue)); }
    //static EventType string_to_event_type(const std::string& eventType);
    struct EventData
    {
        GlobalEnum::ScheduleAction action;
        Value value;
    };
    TimeTable();
    TimeTable(std::vector<ActionData>&& actionList);
    virtual ~TimeTable();

    //Add an event to happen at eventTime
    bool add_write_event(const Value &value, const DayTime &eventTime);
    //Add 2 events, 1 to happen at the begin time, the other to happen at the end time + 1
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
    //All events are invoked based on eventMap
    std::map<uint32_t, EventData> eventMap;
    //Interval map to check whether should set value now
    std::map<uint32_t, IntervalData> intervalMap;

    void add_interval(const Value& value, uint32_t beginSecond, uint32_t endSecond);
};

#endif