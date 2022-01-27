/**************************************************
 * Contains TimeTable and ScheduleRules pair and priority.
 * ScheduleRules would determine if the associated TimeTable is applicable 'today'.
 * Priority would determine which of the applicable TimeTable would apply.
 * Follow the default TimeTable if no TimeTable is applicable AND if default TimeTable exist.
 * Always redetermine which TimeTable to follow at 00:00:00
***************************************************/
#ifndef _Schedule_H_
#define _Schedule_H_
#include <memory>
#include <map>
#include <unordered_map>
#include "../../MyLib/Timer/EventTimer.h"
#include "ScheduleRule.h"
#include "TimeTable.h"
#include "../../MyLib/Basic/Value.h"

class Schedule {
public:
    class Listener {
    public:
        Listener() {}
        virtual ~Listener() {}
        virtual void catch_set_event(const Value& setValue) {}
        virtual void catch_unset_event() {}
        virtual void catch_write_event(const Value& setValue) {}
    };
    class TimerListener : public EventTimer::Listener {
    public:
        TimerListener(Schedule& _master) : master(_master) {}
        virtual ~TimerListener() {}
        void catch_time_event(time_t eventTime, uint32_t token) { master.catch_time_event(eventTime, token); }
    private:
        Schedule& master;
    };

    Schedule(std::shared_ptr<TimeTable> _defaultTimeTable);
    virtual ~Schedule();

    //void set_default_time_table(std::shared_ptr<TimeTable> timeTable);
    //priority follow std::map, lower 1st.
    void add_time_table(std::shared_ptr<TimeTable> timeTable, std::shared_ptr<ScheduleRule> rules, uint8_t priority);
    void add_listener(std::weak_ptr<Listener> listener);
    void start();

    //Interface function to Timer
    void catch_time_event(time_t eventTime, uint32_t token);

private:
    struct TimeTableData {
        std::shared_ptr<ScheduleRule> scheduleRule;
        std::shared_ptr<TimeTable> timeTable;
    };
    std::shared_ptr<TimerListener> timerListener;
    //std::shared_ptr<Timer>& timer;
    std::unordered_map<void*, std::weak_ptr<Listener>> listenerMap;
    std::map<uint8_t, TimeTableData> priority2TimeTableMap;
    std::shared_ptr<TimeTable> defaultTimeTable = nullptr;
    std::shared_ptr<TimeTable> activeTimeTable = nullptr;

    //Find is based on today's date
    std::shared_ptr<TimeTable> get_applicable_time_table();

    //void handle_event(const TimeTable::EventData& eventInfo);
    void notify_listener_set_event(const Value& setValue);
    void notify_listener_unset_event();
    void notify_listener_write_event(const Value& setValue);
};

#endif