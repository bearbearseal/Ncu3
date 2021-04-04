#include "ScheduleManager.h"

using namespace std;

ScheduleManager::ScheduleManager(ConfigStorage &_configStorage, shared_ptr<VariableTree> equipmentContainer) : configStorage(_configStorage)
{
    timer = make_unique<Timer>();
    auto conditionMap = load_schedule_condition();
    auto ruleMap = load_schedule_rules(conditionMap);
    auto timeTableMap = load_time_table();
    auto scheduleMap = load_schedule_map(ruleMap, timeTableMap);
    scheduleDataMap = create_schedule_data_map(equipmentContainer, scheduleMap);
}

//All the shared_ptr keep a reference to ScheduleManager, make sure they r deleted b4 ScheduleManager is deleted
ScheduleManager::~ScheduleManager()
{
    //Free all shared_ptr
    for (auto i = scheduleDataMap.begin(); i != scheduleDataMap.end(); ++i)
    {
        i->second.scheduleListener.reset();
    }
    //Wait for all shared_ptr to be free
    for (auto i = scheduleDataMap.begin(); i != scheduleDataMap.end(); ++i)
    {
        weak_ptr<ScheduleListener> weak = i->second.scheduleListener;
        while (weak.lock() != nullptr)
            ;
    }
}

void ScheduleManager::start()
{
    for (auto i = scheduleDataMap.begin(); i != scheduleDataMap.end(); ++i)
    {
        i->second.schedule->start();
    }
}

void ScheduleManager::catch_set_event(uint16_t id, const Value &setValue)
{
    auto listenerData = scheduleDataMap.find(id);
    if (listenerData == scheduleDataMap.end())
    {
        return;
    }
    for (auto i = listenerData->second.triggerList.begin(); i != listenerData->second.triggerList.end();)
    {
        auto shared = i->lock();
        if (shared != nullptr)
        {
            shared->set_value(setValue, SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = listenerData->second.triggerList.erase(i);
        }
    }
}

void ScheduleManager::catch_unset_event(uint16_t id)
{
    auto listenerData = scheduleDataMap.find(id);
    if (listenerData == scheduleDataMap.end())
    {
        return;
    }
    for (auto i = listenerData->second.triggerList.begin(); i != listenerData->second.triggerList.end();)
    {
        auto shared = i->lock();
        if (shared != nullptr)
        {
            shared->unset_value(SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = listenerData->second.triggerList.erase(i);
        }
    }
}

void ScheduleManager::catch_write_event(uint16_t id, const Value &writeValue)
{
    auto listenerData = scheduleDataMap.find(id);
    if (listenerData == scheduleDataMap.end())
    {
        return;
    }
    for (auto i = listenerData->second.triggerList.begin(); i != listenerData->second.triggerList.end();)
    {
        auto shared = i->lock();
        if (shared != nullptr)
        {
            shared->write_value(writeValue, SCHEDULE_CONTROL_PRIORITY);
            ++i;
        }
        else
        {
            i = listenerData->second.triggerList.erase(i);
        }
    }
}

unordered_map<uint16_t, vector<ScheduleRule::Condition>> ScheduleManager::load_schedule_condition()
{
    unordered_map<uint16_t, vector<ScheduleRule::Condition>> retVal;
    auto conditionData = configStorage.get_schedule_condition_data();
    printf("Schedule manager loading condition.\n");
    for (auto i = conditionData.begin(); i != conditionData.end(); ++i)
    {
        vector<ScheduleRule::Condition> entry;
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            ScheduleRule::Condition condition;
            condition.subject = ScheduleRule::string_to_subject(i->second[j].subject);
            condition.compare = ScheduleRule::string_to_comparison(i->second[j].comparison);
            if (ScheduleRule::subject_is_valid(condition.subject) && ScheduleRule::comparison_is_valid(condition.compare))
            {
                condition.value = i->second[j].value;
                printf("Creating schedule condition: %s %s %u\n", i->second[j].subject.c_str(), i->second[j].comparison.c_str(), i->second[j].value);
                entry.push_back(condition);
            }
        }
        if (entry.size())
        {
            printf("Add to condition id %u\n", i->first);
            retVal[i->first] = move(entry);
        }
    }
    return retVal;
}

std::unordered_map<uint16_t, std::shared_ptr<ScheduleRule>> ScheduleManager::load_schedule_rules(unordered_map<uint16_t, vector<ScheduleRule::Condition>> &id2ConditionMap)
{
    printf("Schedule manager loading schedule rule.\n");
    unordered_map<uint16_t, shared_ptr<ScheduleRule>> retVal;
    auto rule2Conditions = configStorage.get_schedule_rule_and_condition_data();
    for (auto i = rule2Conditions.begin(); i != rule2Conditions.end(); ++i)
    {
        shared_ptr<ScheduleRule> entry = make_shared<ScheduleRule>();
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            if (id2ConditionMap.count(i->second[j]))
            {
                printf("Schedule rule add condition id %u\n", i->second[j]);
                entry->add_conditions(id2ConditionMap[i->second[j]]);
            }
        }
        printf("Schedule %u created.\n", i->first);
        retVal[i->first] = entry;
    }
    return retVal;
}

std::unordered_map<uint16_t, std::shared_ptr<TimeTable>> ScheduleManager::load_time_table()
{
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table_data();
    for (auto i = timeTableData.begin(); i != timeTableData.end(); ++i)
    {
        shared_ptr<TimeTable> entry = make_shared<TimeTable>();
        printf("Time table created: %p\n", entry.get());
        TimeTable::DayTime intervalStartTime;
        Value intervalValue;
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            printf("In time table loop.\n");
            Value theValue;
            theValue.from_string(i->second[j].valueString);
            TimeTable::EventType eventType = TimeTable::string_to_event_type(i->second[j].eventType);
            switch (eventType)
            {
            case TimeTable::EventType::StartInterval:
                intervalStartTime.hour = i->second[j].hour;
                intervalStartTime.minute = i->second[j].minute;
                intervalStartTime.second = i->second[j].second;
                intervalValue = theValue;
                printf("Creating an interval start at %02u:%02u:%02u value: %s.\n", intervalStartTime.hour, intervalStartTime.minute, intervalStartTime.second, intervalValue.to_string().c_str());
                break;
            case TimeTable::EventType::EndInterval:
                if (!intervalValue.is_empty())
                {
                    printf("Creating interval with end time at %02u:%02u:%02u.\n", i->second[j].hour, i->second[j].minute, i->second[j].second);
                    entry->add_interval(intervalValue, intervalStartTime, {i->second[j].hour, i->second[j].minute, i->second[j].second});
                    intervalValue.delete_data();
                }
                else
                {
                    printf("Cannot create end with no start.\n");
                }
                break;
            case TimeTable::EventType::WriteValue:
                printf("Creating write event at time %02u:%02u:%02u, value:%s.\n", i->second[j].hour, i->second[j].minute, i->second[j].second, theValue.to_string().c_str());
                entry->add_write_event(theValue, {i->second[j].hour, i->second[j].minute, i->second[j].second});
                break;
            case TimeTable::EventType::Invalid:
                printf("Invalid event type.\n");
                break;
            }
        }
        retVal[i->first] = entry;
    }
    return retVal;
}

std::unordered_map<uint16_t, std::unique_ptr<Schedule>> ScheduleManager::load_schedule_map(unordered_map<uint16_t, shared_ptr<ScheduleRule>> &scheduleRuleMap, unordered_map<uint16_t, shared_ptr<TimeTable>> &timeTableMap)
{
    unordered_map<uint16_t, unique_ptr<Schedule>> scheduleMap;
    auto scheduleData = configStorage.get_schedule_data();
    for (auto i = scheduleData.begin(); i != scheduleData.end(); ++i)
    {
        unique_ptr<Schedule> &theSchedule = scheduleMap[i->first];
        theSchedule = make_unique<Schedule>(timer);
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            auto ruleIter = scheduleRuleMap.find(i->second[j].ruleId);
            auto timeTableIter = timeTableMap.find(i->second[j].timeTableId);
            if (ruleIter != scheduleRuleMap.end() && timeTableIter != timeTableMap.end())
            {
                theSchedule->add_time_table(timeTableIter->second, ruleIter->second, i->second[j].priority);
            }
        }
    }
    return scheduleMap;
}

unordered_map<uint16_t, ScheduleManager::ScheduleData> ScheduleManager::create_schedule_data_map(shared_ptr<VariableTree> equipmentContainer, unordered_map<uint16_t, unique_ptr<Schedule>> &scheduleMap)
{
    unordered_map<uint16_t, ScheduleManager::ScheduleData> retVal;
    auto configData = configStorage.get_equipment_schedule_data();
    for (size_t i = 0; i < configData.size(); ++i)
    {
        auto scheduleMapItem = scheduleMap.find(configData[i].scheduleId);
        if (scheduleMapItem == scheduleMap.end())
        {
            continue;
        }
        auto equipment = equipmentContainer->get_child(configData[i].equipmentId);
        if (equipment == nullptr)
        {
            continue;
        }
        auto property = equipment->get_child(configData[i].propertyName);
        if (property == nullptr)
        {
            continue;
        }
        ScheduleData &scheduleData = retVal[configData[i].scheduleId];
        if (scheduleData.schedule == nullptr)
        {
            scheduleData.schedule = move(scheduleMapItem->second);
            scheduleData.scheduleListener = make_shared<ScheduleListener>(configData[i].scheduleId, *this);
            scheduleData.schedule->add_listener(scheduleData.scheduleListener);
        }
        scheduleData.triggerList.push_back(property);
    }
    return retVal;
}
