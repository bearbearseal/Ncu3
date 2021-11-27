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
        // Create list of condition list
        vector<vector<ScheduleRule::Condition>> conditionListList;
        auto &theList = i->second;
        for (size_t j = 0; j < theList.size(); ++j)
        {
            // Create condition list
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
            if (conditionList.size())
            {
                conditionListList.push_back(move(conditionList));
            }
        }
        if (conditionListList.size())
        {
            shared_ptr<ScheduleRule> scheduleRule = make_shared<ScheduleRule>(move(conditionListList));
            retVal[i->first] = scheduleRule;
        }
    }
    return retVal;
}

int32_t get_time_from_hhmmss(uint32_t hhmmss)
{
    uint32_t hour = hhmmss / 10000;
    if (hour > 23)
    {
        return -1;
    }
    uint32_t minute = (hhmmss / 100) % 100;
    if (minute > 59)
    {
        return -1;
    }
    uint32_t second = hhmmss % 100;
    if (second > 59)
    {
        return -1;
    }
    uint32_t totalSecond = hour * 3600;
    totalSecond += minute * 60;
    totalSecond += second;
    return static_cast<int32_t>(totalSecond);
}

unordered_map<uint16_t, shared_ptr<TimeTable>> ScheduleManager::load_time_table()
{
    unordered_map<uint16_t, shared_ptr<TimeTable>> retVal;
    auto timeTableData = configStorage.get_time_table();
    for (size_t i = 0; i < timeTableData.size(); ++i)
    {
        vector<TimeTable::TimedActionData> timedActionList;
        auto &actionList = timeTableData[i].actionList;
        for (size_t j = 0; j < actionList.size(); ++j)
        {
            int32_t daySecond = get_time_from_hhmmss(actionList[j].time);
            if (GlobalEnum::is_schedule_action(actionList[j].action) && daySecond >= 0)
            {
                timedActionList.push_back({actionList[j].time, static_cast<GlobalEnum::ScheduleAction>(actionList[j].action), actionList[j].value});
            }
            else
            {
                break;
            }
        }
        //shared_ptr<TimeTable> timeTable = make_shared<TimeTable>(timedActionList);
        retVal[timeTableData[i].id] = make_shared<TimeTable>(timedActionList);
    }
    return retVal;
}

unordered_map<uint16_t, unique_ptr<Schedule>> ScheduleManager::load_schedule_map(
    unordered_map<uint16_t, shared_ptr<ScheduleRule>> scheduleRuleMap,
    unordered_map<uint16_t, shared_ptr<TimeTable>> timeTableMap)
{
    unordered_map<uint16_t, unique_ptr<Schedule>> scheduleMap;
    auto scheduleList = configStorage.get_schedule();
    for (size_t i = 0; i < scheduleList.size(); ++i)
    {
        unique_ptr<Schedule> schedule;
        if (timeTableMap.contains(scheduleList[i].defaultTable))
        {
            schedule = make_unique<Schedule>(timer, timeTableMap[scheduleList[i].defaultTable]);
        }
        else
        {
            continue;
        }
        auto &pairList = scheduleList[i].pairList;
        for (size_t j = 0; j < pairList.size(); ++j)
        {
            if (scheduleRuleMap.contains(pairList[j].scheduleRule) && timeTableMap.contains(pairList[j].timeTable))
            {
                schedule->add_time_table(timeTableMap[pairList[j].timeTable], scheduleRuleMap[pairList[j].scheduleRule], j);
            }
        }
    }
    return scheduleMap;
}
