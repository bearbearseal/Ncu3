#ifndef _AlarmLogic_H_
#define _AlarmLogic_H_
#include "AlarmDefinition.h"
#include "AlarmVerifier.h"
#include <optional>

class AlarmLogicGroup {
public:
    struct AlarmData
    {
        AlarmDefinition::Comparison compare;
        Value refValue;
        AlarmDefinition::AlarmState state;
    };
    AlarmLogicGroup(const std::vector<AlarmData>& logicData);
    virtual ~AlarmLogicGroup();
    std::optional<AlarmData> check_alarm(const Value& value);

private:
    std::vector<std::pair<AlarmVerifier, AlarmData>> verifierList;
};

#endif
