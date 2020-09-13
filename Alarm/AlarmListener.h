#ifndef _AlarmListener_H_
#define _AlarmListener_H_
#include <chrono>
#include "../Basic/HashKey.h"
#include "../Basic/Value.h"

class AlarmListener {
public:
    AlarmListener(){}
    virtual ~AlarmListener(){}

    virtual void catch_alarm(const HashKey::EitherKey& source, const Value& leftValue, const Value& rightValue, std::chrono::time_point<std::chrono::system_clock> theMoment, uint32_t condition) = 0;
};

#endif