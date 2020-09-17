#ifndef _AlarmListener_H_
#define _AlarmListener_H_
#include <chrono>
#include "../Basic/HashKey.h"
#include "../Basic/Value.h"
#include "AlarmDefinition.h"

class AlarmListener {
public:
    AlarmListener(){}
    virtual ~AlarmListener(){}

    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage) = 0;
    /*
    virtual void catch_alarm(const HashKey::EitherKey& equipment, const HashKey::EitherKey& source, 
        const Value& leftValue, const HashKey::EitherKey& right, const Value& rightValue, const std::string& message, 
        std::chrono::time_point<std::chrono::system_clock> theMoment, const AlarmDefinition::Condition& condition) = 0;
    */
};

#endif