#ifndef _AlarmHandler_H_
#define _AlarmHandler_H_
#include "AlarmListener.h"
#include "AlarmDefinition.h"

class AlarmHandler : public AlarmListener {
public:
    AlarmHandler();
    virtual ~AlarmHandler();
    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);
    /*
    void catch_alarm(const HashKey::EitherKey& equipment, const HashKey::EitherKey& source, const Value& leftValue, 
        const HashKey::EitherKey& right, const Value& rightValue, const std::string& message, 
        std::chrono::time_point<std::chrono::system_clock> theMoment, const AlarmDefinition::Condition& condition);
    */
};

#endif