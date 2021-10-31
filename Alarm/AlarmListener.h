#ifndef _AlarmListener_H_
#define _AlarmListener_H_
#include "AlarmDefinition.h"
#include <stdint.h>
#include <memory>

class AlarmListener
{
public:
    virtual ~AlarmListener() {}
    virtual void catch_alarm(uint64_t alarmId, const std::unique_ptr<AlarmDefinition::AlarmMessage> &alarmMessage) = 0;
};

#endif