#ifndef _EquipmentScheduleManager_H_
#define _EquipmentScheduleManager_H_
#include "../Schedule/ScheduleManager.h"
#include "../VariableTree/VariableTree.h"
#include "../Storage/ConfigStorage.h"

class EquipmentScheduleManager
{
public:
    static const uint8_t SCHEDULE_CONTROL_PRIORITY = 1;
private:
    class ScheduleListener : public Schedule::Listener
    {
    public:
        void add_subscriber(std::weak_ptr<VariableTree> subscriber);
        void catch_set_event(const Value& setValue);
        void catch_unset_event();
        void catch_write_event(const Value& setValue);
    private:
        std::vector<std::weak_ptr<VariableTree>> subscriberList;
    };

public:
    EquipmentScheduleManager();
    virtual ~EquipmentScheduleManager();

    void load_equipment_property_and_schedule_pair(ConfigStorage& configStorage, std::shared_ptr<VariableTree> equipmentContainer, ScheduleManager& scheduleManager);

private:
    std::unordered_map<uint16_t, std::shared_ptr<ScheduleListener>> scheduleListenerMap;
};

#endif