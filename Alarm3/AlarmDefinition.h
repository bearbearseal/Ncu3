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
    enum AlarmState
    {
        VeryLow = -2,
        Low = -1,
        Normal = 0,
        High = 1,
        VeryHigh = 2,
        OutOfRange = 3
    };
    struct PointId {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
    };
    struct AlarmMessage {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
        AlarmState activeState;
        AlarmState oldState;
        std::string message;
        Value activeValue;
        Value referenceValue;
        Comparison comparison;
        uint32_t code;
        uint64_t milliSecTime;
    };
}