#ifndef _AlarmLogic_H_
#define _AlarmLogic_H_
#include "AlarmDefinition.h"
#include "AlarmStateChecker.h"
#include <optional>

class AlarmLogicGroup {
public:
    struct AlarmData
    {
        AlarmDefinition::Comparison compare;
        Value refValue;
        AlarmDefinition::AlarmState state = AlarmDefinition::AlarmState::NORMAL;
    };
    AlarmLogicGroup() {}
    AlarmLogicGroup(const std::vector<AlarmData>& logicData);
    virtual ~AlarmLogicGroup();
    AlarmData check_alarm(const Value& value) const;

private:
    std::vector<std::pair<AlarmStateChecker, AlarmData>> checkerList;
};

#endif
