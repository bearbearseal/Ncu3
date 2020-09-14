#ifndef _AlarmCondition_H_
#define _AlarmCondition_H_
#include <stdint.h>

namespace AlarmDefinition {
    enum class ConditionType {
        None = 0,
        Alarm = 1,
        Event = 2,
        Error = 3
    };
    enum class ErrorCode {
        PropertyNotFound = 1,
        ObjectDeleted = 2
    };

    struct Condition {
        Condition() {}
        Condition(ConditionType _type, uint32_t _code) { type=_type; code=_code; }
        Condition(const Condition& theOther) { type=theOther.type; code=theOther.code; }
        ConditionType type;
        uint32_t code;
    };
}

#endif