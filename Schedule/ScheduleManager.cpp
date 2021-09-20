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

bool ScheduleManager::schedule_add_listener(uint16_t scheduleId, std::weak_ptr<Schedule::Listener> listener)
{
    auto i = scheduleMap.find(scheduleId);
    if (i != scheduleMap.end())
    {
        i->second->add_listener(listener);
        return true;
    }
    return false;
}

bool ScheduleManager::has_schedule(uint16_t scheduleId)
{
    return bool(scheduleMap.count(scheduleId));
}

unordered_map<uint16_t, shared_ptr<ScheduleRule>> ScheduleManager::load_schedule_rules()
{
    auto conditionData = configStorage.get_schedule_condition_data();
    std::unordered_map<uint16_t, vector<ScheduleRule::Condition>> id2ConditionMap;
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
            id2ConditionMap[i->first] = move(entry);
        }
    }
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

unordered_map<uint16_t, shared_ptr<TimeTable>> ScheduleManager::load_time_table()
{
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table_data();
    for (auto i = timeTableData.begin(); i != timeTableData.end(); ++i)
    {
        shared_ptr<TimeTable> entry = make_shared<TimeTable>();
        //printf("Time table created: %p\n", entry.get());
        TimeTable::DayTime intervalStartTime;
        Value intervalValue;
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            //printf("In time table loop.\n");
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
                    //printf("Creating an interval start at %02u:%02u:%02u value: %s.\n", intervalStartTime.hour, intervalStartTime.minute, intervalStartTime.second, intervalValue.to_string().c_str());
                    break;
                case TimeTable::EventType::EndInterval:
                    if (!intervalValue.is_empty())
                    {
                        //printf("Creating interval with end time at %02u:%02u:%02u.\n", i->second[j].hour, i->second[j].minute, i->second[j].second);
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
