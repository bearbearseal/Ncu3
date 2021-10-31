#ifndef _AlarmStore_H_
#define _AlarmStore_H_
#include <stdint.h>
#include <vector>
#include "AlarmDefinition.h"

class AlarmStore
{
private:
    const static size_t MAX_Alarm_Per_Message = 1000;
public:
    struct ReadAlarmData
    {
        std::vector<std::pair<uint64_t, AlarmDefinition::AlarmMessage>> alarmList;
        uint64_t continuation = 0;
    };
    virtual ~AlarmStore() {}
    virtual ReadAlarmData read_active_alarm() = 0;
    virtual ReadAlarmData read_active_alarm(uint64_t id) = 0;
    virtual ReadAlarmData read_history_alarm_id(uint64_t id, size_t count = MAX_Alarm_Per_Message) = 0;
    virtual ReadAlarmData read_history_alarm_count(uint64_t beginTime, size_t count = MAX_Alarm_Per_Message) = 0;
    virtual ReadAlarmData read_history_alarm_interval(uint64_t beginTime, uint64_t endTime) = 0;
};

#endif