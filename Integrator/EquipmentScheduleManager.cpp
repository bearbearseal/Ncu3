#include "EquipmentScheduleManager.h"

using namespace std;

EquipmentScheduleManager::EquipmentScheduleManager()
{

}

EquipmentScheduleManager::~EquipmentScheduleManager()
{

}

void EquipmentScheduleManager::load_equipment_property_and_schedule_pair(ConfigStorage& configStorage, std::shared_ptr<VariableTree> equipmentContainer, ScheduleManager& scheduleManager)
{
    auto configData = configStorage.get_equipment_schedule_data();
    for(size_t i=0; i<configData.size(); ++i)
    {
        if(scheduleManager.has_schedule(configData[i].scheduleId))
        {
            continue;
        }
        auto equipment = equipmentContainer->get_child(configData[i].equipmentId);
        if(equipment == nullptr)
        {
            continue;
        }
        auto property = equipment->get_child(configData[i].propertyName);
        if(property == nullptr)
        {
            continue;
        }
        shared_ptr<ScheduleListener>& scheduleListener = scheduleListenerMap[configData[i].scheduleId];
        scheduleListener = make_shared<ScheduleListener>();
        scheduleListener->add_subscriber(property);
        scheduleManager.schedule_add_listener(configData[i].scheduleId, scheduleListener);
    }
}

void EquipmentScheduleManager::ScheduleListener::add_subscriber(std::weak_ptr<VariableTree> subscriber)
{
    subscriberList.push_back(subscriber);
}

void EquipmentScheduleManager::ScheduleListener::catch_set_event(const Value& setValue)
{
    for(auto i = subscriberList.begin(); i != subscriberList.end();)
    {
        auto shared = i->lock();
        if(shared != nullptr)
        {
            shared->set_value(setValue, SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = subscriberList.erase(i);
        }
    }
}

void EquipmentScheduleManager::ScheduleListener::catch_unset_event()
{
    for(auto i = subscriberList.begin(); i != subscriberList.end();)
    {
        auto shared = i->lock();
        if(shared != nullptr)
        {
            shared->unset_value(SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = subscriberList.erase(i);
        }
    }
}

void EquipmentScheduleManager::ScheduleListener::catch_write_event(const Value& setValue)
{
    for(auto i = subscriberList.begin(); i != subscriberList.end();)
    {
        auto shared = i->lock();
        if(shared != nullptr)
        {
            shared->write_value(setValue, SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = subscriberList.erase(i);
        }
    }
}
