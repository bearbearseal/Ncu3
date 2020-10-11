#pragma once
#include <memory>
#include "AlarmDefinition.h"

class AlarmLogicConstant {
public:
    AlarmLogicConstant(AlarmDefinition::Comparison compare, const Value& rightValue, const std::string& message, AlarmDefinition::Condition condition);
    ~AlarmLogicConstant();

    bool condition_hit(const Value& leftValue);
    const std::string& get_message();
    const AlarmDefinition::Condition& get_condition();
    const Value& get_right_value();

private:
    const AlarmDefinition::Comparison compare;
    const Value rightValue;
    const std::string message;
    const AlarmDefinition::Condition condition;
    //const AlarmDefinition::ConditionType type;
    //const uint32_t code;
};