#ifndef _AlarmCondition_H_
#define _AlarmCondition_H_
#include <stdint.h>
#include <string>
#include "../Basic/Value.h"
//#include <iostream>
//#include <sstream>

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
        bool operator==(const Condition& theOther) { return ((type == theOther.type) && (code == theOther.code));}
        bool operator!=(const Condition& theOther) { return ((type != theOther.type) || (code != theOther.code));}
        std::string to_string() const {
            char buffer[32];
            switch(type) {
                case ConditionType::None:
                    snprintf(buffer, sizeof(buffer), "None");
                    break;
                case ConditionType::Alarm:
                    snprintf(buffer, sizeof(buffer), "Alarm %u", code);
                    break;
                case ConditionType::Event:
                    snprintf(buffer, sizeof(buffer), "Event %u", code);
                    break;
                case ConditionType::Error:
                    snprintf(buffer, sizeof(buffer), "Error %u", code);
                    break;
            }
            return buffer;
        }
        
        ConditionType type;
        uint32_t code;
    };

    struct AlarmMessage {
        const HashKey::EitherKey equipment;
        const HashKey::EitherKey source; 
        const Value leftValue;
        const HashKey::EitherKey right;
        const Value rightValue;
        const std::string message; 
        const std::chrono::time_point<std::chrono::system_clock> theMoment;
        const AlarmDefinition::Condition condition;
    };
}

#endif