#ifndef _AlarmListener_H_
#define _AlarmListener_H_
#include <chrono>
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/Basic/Value.h"
#include "AlarmDefinition.h"

class AlarmListener {
public:
    AlarmListener(){}
    virtual ~AlarmListener(){}

    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage) = 0;
};

#endif