#ifndef _AlarmDefinition_H_
#define _AlarmDefinition_H_
#include <string>
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/Basic/Value.h"

namespace AlarmDefinition {
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
        VERY_LOW = -2,
        LOW = -1,
        NORMAL = 0,
        HIGH = 1,
        VERY_HIGH = 2,
        OUT_OF_RANGE = 3
    };
    struct PointId {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
    };
    struct AlarmMessage {
        AlarmMessage() {}
        AlarmMessage(
            HashKey::EitherKey _equipmentId, 
            HashKey::EitherKey _propertyId, 
            AlarmState _activeState, 
            AlarmState _oldState, 
            Value _activeValue, 
            Value _referenceValue, 
            Comparison _comparison, 
            uint64_t _milliSecTime) :
            equipmentId(_equipmentId),
            propertyId(_propertyId),
            activeState(_activeState),
            oldState(_oldState),
            activeValue(_activeValue),
            referenceValue(_referenceValue),
            comparison(_comparison),
            milliSecTime(_milliSecTime)
        {}
        AlarmMessage(AlarmMessage&& theOther)
        {
            equipmentId = std::move(theOther.equipmentId);
            propertyId = std::move(theOther.propertyId);
            activeState = theOther.activeState;
            oldState = theOther.oldState;
            activeValue = std::move(theOther.activeValue);
            referenceValue = std::move(theOther.referenceValue);
            comparison = theOther.comparison;
            milliSecTime = theOther.milliSecTime;
        }
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
        AlarmState activeState;
        AlarmState oldState;
        Value activeValue;
        Value referenceValue;
        Comparison comparison;
        uint64_t milliSecTime;
    };
}

#endif