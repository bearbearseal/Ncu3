#ifndef _TimeTable_H_
#define _TimeTable_H_
#include <map>
#include <optional>
#include "../Global/GlobalEnum.h"
#include "../../MyLib/Basic/Value.h"

class TimeTable {
public:
    struct TimedActionData {
        uint32_t daySecond;
        GlobalEnum::ScheduleAction action;
        Value value;
    };
    struct ActionData {
        GlobalEnum::ScheduleAction action;
        Value value;
    };

    TimeTable(const std::vector<TimedActionData>& timedActionList);
    virtual ~TimeTable();

    //Would get the last event if there is no event b4, as like getting the last event of yesterday
    const ActionData& get_action_at(uint32_t daySecond) const;
    std::optional<TimedActionData> get_event_before_or_at(uint32_t daySecond) const;
    std::optional<TimedActionData> get_the_event_after(uint32_t daySecond) const;
    std::optional<TimedActionData> get_the_event_at_or_after(uint32_t daySecond) const;
    std::optional<TimedActionData> get_first_event() const;

private:
    std::map<uint32_t, ActionData> orderedActions;
};

#endif