#ifndef _AlarmDefinition_H_
#define _AlarmDefinition_H_
#include <string>
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/Basic/Value.h"

namespace AlarmDefinition {
    enum class Comparison : uint8_t
    {
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
    struct AlarmMessage
    {
        AlarmMessage() : deviceId(0), pointId(0), comparison(AlarmDefinition::Comparison::EQUAL),
                         alarmState(AlarmDefinition::AlarmState::NORMAL), msTime(0) {}
        AlarmMessage(const AlarmMessage &theOther) : deviceId(theOther.deviceId), pointId(theOther.pointId), activeValue(theOther.activeValue),
                                                     refValue(theOther.refValue), comparison(theOther.comparison), alarmState(theOther.alarmState),
                                                     msTime(theOther.msTime) {}
        AlarmMessage(uint32_t _deviceId, uint32_t _pointId, const Value &_activeValue, const Value &_refValue,
                     AlarmDefinition::Comparison _comparison, AlarmDefinition::AlarmState _alarmState, uint64_t _msTime) : deviceId(_deviceId), pointId(_pointId), activeValue(_activeValue), refValue(_refValue),
                                                                                                                           comparison(_comparison), alarmState(_alarmState), msTime(_msTime) {}
        uint32_t deviceId;
        uint32_t pointId;
        Value activeValue;
        Value refValue;
        AlarmDefinition::Comparison comparison;
        AlarmDefinition::AlarmState alarmState;
        uint64_t msTime;
    };
}

#endif