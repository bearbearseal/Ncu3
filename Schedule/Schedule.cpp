#include "Schedule.h"
#include "ScheduleFunction.h"

using namespace std;

const uint16_t TOKEN_Tomolo = 0;
const uint16_t TOKEN_NextEvent = 1;

Schedule::Schedule(unique_ptr<Timer>& _timer) : timer(_timer)
{
    defaultTimeTable = make_shared<TimeTable>();
    timerListener = make_shared<TimerListener>(*this);
}

Schedule::~Schedule()
{
}

void Schedule::set_default_time_table(std::shared_ptr<TimeTable> timeTable)
{
    defaultTimeTable = timeTable;
}

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
        //Set an event, tomolo 00:00:00
        time_t tomolo = today0Second + (24 * 3600);
        timer->add_time_event(tomolo, timerListener, 0); //0 for tomolo event
    }
    else
    {
        printf("Found active time table %p.\n", activeTimeTable.get());
        //Check today date
        auto theInterval = activeTimeTable->get_interval_value(daySec);
        auto theEvent = activeTimeTable->get_the_event_after(daySec);
        //printf("The interval:");
        //printf("Endsecond = %u\n", theInterval.endSecond);
        if (!theInterval.value.is_empty())
        {
            printf("No interval.\n");
            //set value
            this->notify_listener_set_event(theInterval.value);
        }
        else
        {
            printf("Got interval.\n");
            //unset value
            this->notify_listener_unset_event();
        }
        printf("Handling event.\n");
        if (theEvent.second.type == TimeTable::EventType::Invalid)
        {
            printf("No event found.\n");
            //Set an event, tomolo 00:00:00
            time_t tomolo = today0Second + (24 * 3600);
            timer->add_time_event(tomolo, timerListener, 0); //0 for tomolo event
        }
        else
        {
            printf("Got an event.\n");
            //Set an event to be call at theEvent time
            time_t nextEventTime = ScheduleFunction::today_second_to_local_time_t(theEvent.first);
            printf("The event time: %u would be fired %lu seconds later.\n", theEvent.first, nextEventTime);
            timer->add_time_event(nextEventTime, timerListener, 1);
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
    case TOKEN_Tomolo:
        printf("Next is Tomolo event.\n");
        activeTimeTable = get_applicable_time_table();
        if (activeTimeTable == nullptr)
        {
            time_t tomolo = nowInSec - daySec + 24 * 3600;
            timer->add_time_event(tomolo, timerListener, 0); //0 for tomolo event
        }
        else
        {
            const std::pair<uint32_t, TimeTable::EventData> &eventInfo = activeTimeTable->get_first_event();
            if (eventInfo.second.type == TimeTable::EventType::Invalid)
            {
                time_t tomolo = nowInSec - daySec + 24 * 3600;
                timer->add_time_event(tomolo, timerListener, TOKEN_Tomolo); //0 for tomolo event
            }
            else if (!eventInfo.first)
            {
                //execute the event now, since the daySecond is 0
                catch_time_event(0, TOKEN_NextEvent);
            }
            else
            {
                time_t eventTime = nowInSec - daySec + eventInfo.first;
                timer->add_time_event(eventTime, timerListener, TOKEN_NextEvent);
            }
        }
        break;
    case TOKEN_NextEvent:
    {
        uint32_t eventSec = ScheduleFunction::get_day_second_of(eventTime);
        const TimeTable::EventData &eventInfo = activeTimeTable->get_event_data(eventSec);
        printf("Caught a timer event, value: %s.\n", eventInfo.value.to_string().c_str());
        switch (eventInfo.type)
        {
        case TimeTable::EventType::StartInterval:
            notify_listener_set_event(eventInfo.value);
            break;
        case TimeTable::EventType::EndInterval:
            notify_listener_unset_event();
            break;
        case TimeTable::EventType::WriteValue:
            notify_listener_write_event(eventInfo.value);
            break;
        case TimeTable::EventType::Invalid:
            //wtf?!
            break;
        }
        time_t today0Sec = ScheduleFunction::today_hms_to_local_time_t(0,0,0);
        const std::pair<uint32_t, TimeTable::EventData> &nextEventInfo = activeTimeTable->get_the_event_after(eventTime-today0Sec);
        if (nextEventInfo.second.type == TimeTable::EventType::Invalid)
        {
            time_t tomolo = today0Sec + (24 * 3600);
            printf("tomolo Sec = %ld\n", tomolo);
            timer->add_time_event(tomolo, timerListener, TOKEN_Tomolo); //0 for tomolo event
        }
        else
        {
            time_t nextEventTime = nowInSec - daySec + nextEventInfo.first;
            printf("Now sec %lu day sec %u next sec %u event Sec %lu\n", nowInSec, daySec, nextEventInfo.first, nextEventTime);
            timer->add_time_event(nextEventTime, timerListener, TOKEN_NextEvent);
        }
        break;
    }

    default:
        //wtf!?
        break;
    }
}

void Schedule::handle_event(const TimeTable::EventData &eventInfo)
{
    switch (eventInfo.type)
    {
    case TimeTable::EventType::StartInterval:
        notify_listener_set_event(eventInfo.value);
        break;
    case TimeTable::EventType::EndInterval:
        notify_listener_unset_event();
        break;
    case TimeTable::EventType::WriteValue:
        notify_listener_write_event(eventInfo.value);
        break;
    case TimeTable::EventType::Invalid:
        //wtf?!
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
    //Check today date
    auto timePoint = chrono::system_clock::now();
    time_t timePoint_t = chrono::system_clock::to_time_t(timePoint);
    struct tm timeStruct;
    localtime_r(&timePoint_t, &timeStruct);
    for (auto i = priority2TimeTableMap.begin(); i != priority2TimeTableMap.end(); ++i)
    {
        if (i->second.scheduleRule->applicable(timeStruct))
        {
            //printf("would follow time table %p\n", i->second.timeTable.get());
            return i->second.timeTable;
        }
    }
    //printf("would follow default time table %p\n", defaultTimeTable.get());
    return defaultTimeTable;
}
