#ifndef _EquipmentScheduleManager_H_
#define _EquipmentScheduleManager_H_
#include "../Schedule/Schedule.h"
#include "../VariableTree/VariableTree.h"
#include "../Storage/ConfigStorage.h"

//Suppose to assign a tree node with specific schedule

class ScheduleManager
{
    friend class ScheduleListener;

public:
    static const uint8_t SCHEDULE_CONTROL_PRIORITY = 1;

private:
    class ScheduleListener : public Schedule::Listener
    {
    public:
        ScheduleListener(size_t _myId, ScheduleManager &_parent) : parent(_parent) { myId = _myId; }
        void add_subscriber(std::weak_ptr<VariableTree> subscriber);
        void catch_set_event(const Value &setValue) { parent.catch_set_event(myId, setValue); }
        void catch_unset_event() { parent.catch_unset_event(myId); }
        void catch_write_event(const Value &writeValue) { parent.catch_write_event(myId, writeValue); }

    private:
        uint16_t myId;
        ScheduleManager &parent;
    };

public:
    ScheduleManager(ConfigStorage &_configStorage, std::shared_ptr<VariableTree> equipmentContainer);
    virtual ~ScheduleManager();

    void start();

    void catch_set_event(uint16_t id, const Value &setValue);
    void catch_unset_event(uint16_t id);
    void catch_write_event(uint16_t id, const Value &writeValue);

private:
    struct ScheduleData
    {
        std::unique_ptr<Schedule> schedule;
        std::shared_ptr<ScheduleListener> scheduleListener;
        std::vector<std::weak_ptr<VariableTree>> triggerList;
    };
    // All the shared pointer inside shall be destroyed b4 ScheduleManager is destroyed
    ConfigStorage& configStorage;
    std::unique_ptr<Timer> timer;
    std::unordered_map<uint16_t, ScheduleData> scheduleDataMap;

    std::unordered_map<uint16_t, std::vector<ScheduleRule::Condition>> load_schedule_condition();
    std::unordered_map<uint16_t, std::shared_ptr<ScheduleRule>> load_schedule_rules(std::unordered_map<uint16_t, std::vector<ScheduleRule::Condition>>& id2ConditionMap);
    std::unordered_map<uint16_t, std::shared_ptr<TimeTable>> load_time_table();
    std::unordered_map<uint16_t, std::unique_ptr<Schedule>> load_schedule_map(std::unordered_map<uint16_t, std::shared_ptr<ScheduleRule>>& scheduleRuleMap, std::unordered_map<uint16_t, std::shared_ptr<TimeTable>>& timeTableMap);
    std::unordered_map<uint16_t, ScheduleData> create_schedule_data_map(std::shared_ptr<VariableTree> equipmentContainer, std::unordered_map<uint16_t, std::unique_ptr<Schedule>>& scheduleMap);
};

#endif