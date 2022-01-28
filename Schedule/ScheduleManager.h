/*********************************************
 * Keeps a collections of Schedule
 * Listener can listen for a Schedule identified by id
 ********************************************/ 

#ifndef _ScheduleManager_H_
#define _ScheduleManager_H_
#include "Schedule.h"
#include "../Storage/ConfigStorage.h"
#include "../VariableTree/VariableTree.h"

class ScheduleManager
{
public:
    ScheduleManager(ConfigStorage& _configStorage);
    virtual ~ScheduleManager();

    void start();
    bool schedule_add_listener(uint16_t scheduleId, std::weak_ptr<Schedule::Listener> listener);
    bool has_schedule(uint16_t scheduleId);

private:
    std::unordered_map<uint16_t, std::shared_ptr<ScheduleRule>> load_schedule_rules();
    std::unordered_map<uint16_t, std::shared_ptr<TimeTable>> load_time_table();
    std::unordered_map<uint16_t, std::unique_ptr<Schedule>> load_schedule_map(std::unordered_map<uint16_t, std::shared_ptr<ScheduleRule>> scheduleRuleMap, std::unordered_map<uint16_t, std::shared_ptr<TimeTable>> timeTableMap);

    ConfigStorage& configStorage;
    //std::shared_ptr<Timer> timer;
    std::unordered_map<uint16_t, std::unique_ptr<Schedule>> scheduleMap;
};

#endif