#include "Schedule.h"
#include "ScheduleFunction.h"

using namespace std;

const uint16_t TOKEN_Tomorrow = 0;
const uint16_t TOKEN_NextEvent = 1;

Schedule::Schedule(unique_ptr<Timer> &_timer, std::shared_ptr<TimeTable> _defaultTimeTable) : timer(_timer), defaultTimeTable(_defaultTimeTable)
{
    timerListener = make_shared<TimerListener>(*this);
}

Schedule::~Schedule()
{
}
/*
void Schedule::set_default_time_table(std::shared_ptr<TimeTable> timeTable)
{
    defaultTimeTable = timeTable;
}
*/
void Schedule::add_time_table(std::shared_ptr<TimeTable> timeTable, std::shared_ptr<ScheduleRule> rules, uint8_t priority)
{
    TimeTableData &entry = priority2TimeTableMap[priority];
    entry.timeTable = timeTable;
    entry.scheduleRule = rules;
}

void Schedule::add_listener(std::weak_ptr<Listener> listener)
{
    auto shared = listener.lock();
    if (shared != nullptr)
    {
        listenerMap[shared.get()] = listener;
    }
}

void Schedule::start()
{
    time_t nowInSec = chrono::system_clock::to_time_t(chrono::system_clock::now());
    uint32_t daySec = ScheduleFunction::get_day_second_of(nowInSec);
    time_t today0Second = ScheduleFunction::today_hms_to_local_time_t(0, 0, 0);
    activeTimeTable = get_applicable_time_table();
    if (activeTimeTable == nullptr)
    {
        printf("Active time table is null.\n");
        // Set an event, tomolo 00:00:00
        time_t tomolo = today0Second + (24 * 3600);
        timer->add_time_event(tomolo, timerListener, TOKEN_Tomorrow);
    }
    else
    {
        auto actionBefore = activeTimeTable->get_event_before_or_at(daySec);
        auto actionAfter = activeTimeTable->get_the_event_after(daySec);
        // If action b4 this is a set event, then execute the set
        if (actionBefore.has_value() && actionBefore->action == GlobalEnum::ScheduleAction::SET)
        {
            this->notify_listener_set_event(actionBefore->value);
        }
        if (actionAfter.has_value())
        {
            time_t nextEventTime = ScheduleFunction::today_second_to_local_time_t(actionAfter->daySecond);
            timer->add_time_event(nextEventTime, timerListener, TOKEN_NextEvent);
        }
        else
        {
            time_t tomorrow = today0Second + (24 * 3600);
            timer->add_time_event(tomorrow, timerListener, TOKEN_Tomorrow);
        }
    }
}

void Schedule::catch_time_event(time_t eventTime, uint32_t token)
{
    printf("Schedule caught event.\n");
    time_t nowInSec = chrono::system_clock::to_time_t(chrono::system_clock::now());
    uint32_t daySec = ScheduleFunction::get_day_second_of(nowInSec);
    switch (token)
    {
    case TOKEN_Tomorrow:
        printf("Next is Tomolo event.\n");
        activeTimeTable = get_applicable_time_table();
        if (activeTimeTable == nullptr)
        {
            time_t tomolo = nowInSec - daySec + 24 * 3600;
            timer->add_time_event(tomolo, timerListener, 0); // 0 for tomolo event
        }
        else
        {
            auto firstAction = activeTimeTable->get_first_event();
            if (firstAction.has_value())
            {
                // add the action
                time_t eventTime = nowInSec - daySec + firstAction->daySecond;
                timer->add_time_event(eventTime, timerListener, TOKEN_NextEvent);
            }
            else
            {
                // tomolo again
                time_t tomorrow = nowInSec - daySec + 24 * 3600;
                timer->add_time_event(tomorrow, timerListener, TOKEN_Tomorrow);
            }
        }
        break;
    case TOKEN_NextEvent:
    {
        uint32_t eventSec = ScheduleFunction::get_day_second_of(eventTime);
        const TimeTable::ActionData& theEvent = activeTimeTable->get_action_at(eventSec);
        if (theEvent.action != GlobalEnum::ScheduleAction::INVALID)
        {
            switch (theEvent.action)
            {
            case GlobalEnum::ScheduleAction::SET:
                notify_listener_set_event(theEvent.value);
                break;
            case GlobalEnum::ScheduleAction::UNSET:
                notify_listener_unset_event();
                break;
            case GlobalEnum::ScheduleAction::WRITE:
                notify_listener_write_event(theEvent.value);
                break;
            case GlobalEnum::ScheduleAction::INVALID:
                break;
            }
        }
        auto nextEvent = activeTimeTable->get_the_event_after(eventSec);
        time_t today0Second = ScheduleFunction::today_hms_to_local_time_t(0, 0, 0);
        if (nextEvent.has_value())
        {
            timer->add_time_event(today0Second + nextEvent->daySecond, timerListener, TOKEN_NextEvent);
        }
        else
        {
            timer->add_time_event(today0Second + (24*3600), timerListener, TOKEN_Tomorrow);
        }
    }

    default:
        // wtf!?
        break;
    }
}

void Schedule::notify_listener_set_event(const Value &setValue)
{
    for (auto i = listenerMap.begin(); i != listenerMap.end();)
    {
        auto shared = i->second.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listenerMap.erase(temp);
        }
        else
        {
            shared->catch_set_event(setValue);
            ++i;
        }
    }
}

void Schedule::notify_listener_unset_event()
{
    printf("Notifying listener.\n");
    for (auto i = listenerMap.begin(); i != listenerMap.end();)
    {
        printf("In loop.\n");
        auto shared = i->second.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listenerMap.erase(temp);
        }
        else
        {
            shared->catch_unset_event();
            ++i;
        }
    }
}

void Schedule::notify_listener_write_event(const Value &setValue)
{
    for (auto i = listenerMap.begin(); i != listenerMap.end();)
    {
        auto shared = i->second.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listenerMap.erase(temp);
        }
        else
        {
            shared->catch_write_event(setValue);
            ++i;
        }
    }
}

std::shared_ptr<TimeTable> Schedule::get_applicable_time_table()
{
    // Check today date
    auto timePoint = chrono::system_clock::now();
    time_t timePoint_t = chrono::system_clock::to_time_t(timePoint);
    struct tm timeStruct;
    localtime_r(&timePoint_t, &timeStruct);
    for (auto i = priority2TimeTableMap.begin(); i != priority2TimeTableMap.end(); ++i)
    {
        if (i->second.scheduleRule->applicable(timeStruct))
        {
            // printf("would follow time table %p\n", i->second.timeTable.get());
            return i->second.timeTable;
        }
    }
    // printf("would follow default time table %p\n", defaultTimeTable.get());
    return defaultTimeTable;
}
