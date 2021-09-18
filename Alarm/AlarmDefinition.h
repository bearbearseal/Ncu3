#pragma once
#include <string>
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/Basic/Value.h"

namespace AlarmDefinition {
    enum class ConditionType {
        None = 0,
        Alarm = 1,
        Event = 2,
        Error = 3
    };
    enum class Comparison {
        GREATER = 1,
        GREATER_EQUAL = 2,
        EQUAL = 3,
        SMALLER_EQUAL = 4,
        SMALLER = 5,
        NOT_EQUAL = 6
    };
    struct Condition {
        Condition() {}
        Condition(ConditionType _type, uint32_t _code) { type=_type; code=_code; }
        Condition(const Condition& theOther) { type=theOther.type; code=theOther.code; }
        bool operator==(const Condition& theOther) const { return ((type == theOther.type) && (code == theOther.code));}
        bool operator!=(const Condition& theOther) const { return ((type != theOther.type) || (code != theOther.code));}
        std::string type_to_string() const {
            switch(type) {
                case ConditionType::Alarm:
                    return "Alarm";
                case ConditionType::Event:
                    return "Event";
                case ConditionType::Error:
                    return "Error";
                case ConditionType::None:
                default:
                    return "None";
            }
        }
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
    struct PointId {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
    };
    struct AlarmLogicConstant {
        Comparison compare;
        Value rightValue;
        Condition condition;
        std::string message;
    };
}