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
    for (auto i = scheduleMap.begin(); i != scheduleMap.end(); ++i)
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
    unordered_map<uint16_t, shared_ptr<ScheduleRule>> retVal;
    auto result = configStorage.get_schedule_rule();
    for (auto i = result.begin(); i != result.end(); ++i)
    {
        //Create list of condition list
        vector<vector<ScheduleRule::Condition>> conditionListList;
        auto &theList = i->second;
        for (size_t j = 0; j < theList.size(); ++j)
        {
            //Create condition list
            auto &subList = theList[j];
            vector<ScheduleRule::Condition> conditionList;
            for (size_t k = 0; k < subList.size(); ++k)
            {
                if (GlobalEnum::is_schedule_subject(subList[k].subject) && GlobalEnum::is_compare(subList[k].compare))
                {
                    conditionList.push_back({static_cast<GlobalEnum::ScheduleSubject>(subList[k].subject), static_cast<GlobalEnum::Compare>(subList[k].compare), subList[k].value});
                }
                else
                {
                    if (subList[k].subject != static_cast<int>(GlobalEnum::ScheduleSubject::INVALID))
                    {
                        printf("Got rubbish subject in schedule rule group %u\n", i->first);
                    }
                    if (subList[k].compare != static_cast<int>(GlobalEnum::Compare::INVALID))
                    {
                        printf("Got rubbish compare in schedule rule group %u\n", i->first);
                    }
                }
            }
            if(conditionList.size())
            {
                conditionListList.push_back(move(conditionList));
            }
        }
        if(conditionListList.size())
        {
            shared_ptr<ScheduleRule> scheduleRule = make_shared<ScheduleRule>(move(conditionListList));
            retVal[i->first] = scheduleRule;
        }
    }
    return retVal;
}

int32_t get_time_from_hhmmss(uint32_t hhmmss)
{
    uint32_t hour = hhmmss/10000;
    if(hour>23)
    {
        return -1;
    }
    uint32_t minute = (hhmmss/100)%100;
    if(minute>59)
    {
        return -1;
    }
    uint32_t second = hhmmss%100;
    if(second>59)
    {
        return -1;
    }
    uint32_t totalSecond = hour*3600;
    totalSecond += minute*60;
    totalSecond += second;
    return static_cast<int32_t>(totalSecond);
}

//If any of the action is invalid, whole time table would not load.
unordered_map<uint16_t, shared_ptr<TimeTable>> ScheduleManager::load_time_table()
{
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table();
    for(size_t i=0; i<timeTableData.size(); ++i)
    {
        shared_ptr<TimeTable> timeTable = make_shared<TimeTable>();
        auto& actionList = timeTableData[i].actionList;
        for(size_t j=0; j<actionList.size(); ++j)
        {
            int32_t daySecond = get_time_from_hhmmss(actionList[j].time);
            if(GlobalEnum::is_schedule_action(actionList[j].action) && daySecond>=0)
            {
                
            }
            else
            {
                printf("Time table % has invalid action, time table would not be created.\n", timeTableData[i].id);
                break;
            }
        }
    }
/*
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table_data();
    for (auto i = timeTableData.begin(); i != timeTableData.end(); ++i)
    {
        shared_ptr<TimeTable> entry = make_shared<TimeTable>();
        // printf("Time table created: %p\n", entry.get());
        TimeTable::DayTime intervalStartTime;
        Value intervalValue;
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            // printf("In time table loop.\n");
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
                // printf("Creating an interval start at %02u:%02u:%02u value: %s.\n", intervalStartTime.hour, intervalStartTime.minute, intervalStartTime.second, intervalValue.to_string().c_str());
                break;
            case TimeTable::EventType::EndInterval:
                if (!intervalValue.is_empty())
                {
                    // printf("Creating interval with end time at %02u:%02u:%02u.\n", i->second[j].hour, i->second[j].minute, i->second[j].second);
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
*/
}

unordered_map<uint16_t, unique_ptr<Schedule>> ScheduleManager::load_schedule_map(unordered_map<uint16_t, shared_ptr<ScheduleRule>> scheduleRuleMap, unordered_map<uint16_t, shared_ptr<TimeTable>> timeTableMap)
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
