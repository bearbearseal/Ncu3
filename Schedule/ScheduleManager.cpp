#include "ScheduleManager.h"
#include "ScheduleFunction.h"

using namespace std;

ScheduleManager::ScheduleManager(ConfigStorage &_configStorage) : configStorage(_configStorage)
{
    timer = make_shared<Timer>();
    auto ruleMap = load_schedule_rules();
    auto timeTableMap = load_time_table();
    scheduleMap = load_schedule_map(ruleMap, timeTableMap);
}

ScheduleManager::~ScheduleManager()
{
}

void ScheduleManager::start()
{
    for(auto i=scheduleMap.begin(); i!=scheduleMap.end(); ++i)
    {
        i->second->start();
    }
}

void ScheduleManager::schedule_add_listener(uint16_t scheduleId, std::weak_ptr<Schedule::Listener> listener)
{
    auto i = scheduleMap.find(scheduleId);
    if (i != scheduleMap.end())
    {
        i->second->add_listener(listener);
    }
}

unordered_map<uint16_t, shared_ptr<ScheduleRule>> ScheduleManager::load_schedule_rules()
{
    auto conditionData = configStorage.get_schedule_condition_data();
    std::unordered_map<uint16_t, vector<ScheduleRule::Condition>> id2ConditionMap;
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
                entry.push_back(condition);
            }
        }
        if (!entry.size())
        {
            id2ConditionMap[i->first] = move(entry);
        }
    }
    unordered_map<uint16_t, shared_ptr<ScheduleRule>> retVal;
    auto rule2Conditions = configStorage.get_schedule_rule_and_condition_data();
    for (auto i = rule2Conditions.begin(); i != rule2Conditions.end(); ++i)
    {
        shared_ptr<ScheduleRule> entry = make_shared<ScheduleRule>();
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            if (id2ConditionMap.count(i->second[j]))
            {
                entry->add_conditions(id2ConditionMap[i->second[j]]);
            }
        }
        retVal[i->first] = entry;
    }
    return retVal;
}

unordered_map<uint16_t, shared_ptr<TimeTable>> ScheduleManager::load_time_table()
{
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table_data();
    for (auto i = timeTableData.begin(); i != timeTableData.end(); ++i)
    {
        shared_ptr<TimeTable> entry = make_shared<TimeTable>();
        TimeTable::DayTime intervalStartTime;
        Value intervalValue;
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            Value theValue;
            theValue.from_string(i->second[j].valueString);
            TimeTable::EventType eventType = TimeTable::string_to_event_type(i->second[j].eventType);
            switch(eventType)
            {
                case TimeTable::EventType::StartInterval:
                    intervalStartTime.hour = i->second[j].hour;
                    intervalStartTime.minute = i->second[j].minute;
                    intervalStartTime.second = i->second[j].second;
                    intervalValue = theValue;
                    break;
                case TimeTable::EventType::EndInterval:
                    if (!intervalValue.is_empty())
                    {
                        entry->add_interval(intervalValue, intervalStartTime, {i->second[j].hour, i->second[j].minute, i->second[j].second});
                        intervalValue.delete_data();
                    }
                    break;
                case TimeTable::EventType::WriteValue:
                    entry->add_write_event(theValue, {i->second[j].hour, i->second[j].minute, i->second[j].second});
                    break;
                case TimeTable::EventType::Invalid:
                    break;
            }
        }
        retVal[i->first] = entry;
    }
    return retVal;
}

unordered_map<uint16_t, unique_ptr<Schedule>> ScheduleManager::load_schedule_map(unordered_map<uint16_t, shared_ptr<ScheduleRule>> scheduleRuleMap, unordered_map<uint16_t, shared_ptr<TimeTable>> timeTableMap)
{
    unordered_map<uint16_t, unique_ptr<Schedule>> scheduleMap;
    auto scheduleData = configStorage.get_schedule_data();
    for (auto i = scheduleData.begin(); i != scheduleData.end(); ++i)
    {
        unique_ptr<Schedule> &theSchedule = scheduleMap[i->first];
        theSchedule = make_unique<Schedule>(timer);
        for (size_t j=0; j<i->second.size(); ++j)
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
