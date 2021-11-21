#include "TimeTable.h"

using namespace std;

TimeTable::ActionData emptyAction{GlobalEnum::ScheduleAction::INVALID, Value()};

TimeTable::TimeTable(const std::vector<TimedActionData> &timedActionList)
{
    for (size_t i = 0; i < timedActionList.size(); ++i)
    {
        ActionData &entry = orderedActions[timedActionList[i].daySecond];
        entry.action = timedActionList[i].action;
        entry.value = timedActionList[i].value;
    }
}

TimeTable::~TimeTable()
{
}

const TimeTable::ActionData& TimeTable::get_action_at(uint32_t daySecond) const
{
    const auto& theResult = orderedActions.find(daySecond);
    if(theResult != orderedActions.end())
    {
        const ActionData& entry = theResult->second;
        return {entry};
    }
    return emptyAction;
}

std::optional<TimeTable::TimedActionData> TimeTable::get_event_before_or_at(uint32_t daySecond) const
{
    const auto& theResult = orderedActions.lower_bound(daySecond);
    if (theResult != orderedActions.end())
    {
        return optional<TimeTable::TimedActionData>{{theResult->first, theResult->second.action, theResult->second.value}};
    }
    return {};
}

std::optional<TimeTable::TimedActionData> TimeTable::get_the_event_after(uint32_t daySecond) const
{
    const auto theResult = orderedActions.upper_bound(daySecond);
    if (theResult != orderedActions.end())
    {
        return optional<TimeTable::TimedActionData>{{theResult->first, theResult->second.action, theResult->second.value}};
    }
    return {};
}

std::optional<TimeTable::TimedActionData> TimeTable::get_the_event_at_or_after(uint32_t daySecond) const
{
    auto theResult = orderedActions.find(daySecond);
    if (theResult != orderedActions.end())
    {
        return optional<TimeTable::TimedActionData>{{theResult->first, theResult->second.action, theResult->second.value}};
    }
    theResult = orderedActions.upper_bound(daySecond);
    if (theResult != orderedActions.end())
    {
        return optional<TimeTable::TimedActionData>{{theResult->first, theResult->second.action, theResult->second.value}};
    }
    return {};
}

std::optional<TimeTable::TimedActionData> TimeTable::get_first_event() const
{
    const auto& theResult = orderedActions.begin();
    if(theResult != orderedActions.end())
    {
        return optional<TimeTable::TimedActionData>{{theResult->first, theResult->second.action, theResult->second.value}};
    }
    return {};
}
