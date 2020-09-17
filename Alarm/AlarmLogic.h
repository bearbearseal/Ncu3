#ifndef _AlarmLogic_H_
#define _AlarmLogic_H_
#include <memory>
#include <unordered_map>
#include <map>
#include <shared_mutex>
#include <vector>
#include "../VariableTree/VariableTree.h"
#include "../Basic/HashKey.h"
#include "AlarmDefinition.h"

//Check logic with bigger priority number 1st.
class AlarmLogic {
public:
    enum class Comparison {
        GREATER,
        GREATER_EQUAL,
        EQUAL,
        SMALLER_EQUAL,
        SMALLER,
        NOT_EQUAL
    };
    AlarmLogic();
    virtual ~AlarmLogic();

    void add_logic(const HashKey::EitherKey& leftProperty, const HashKey::EitherKey& rightProperty2, Comparison compare, uint32_t condition, const std::string& message, uint8_t priority);
    void add_logic(const HashKey::EitherKey& leftProperty1, const Value& constant, Comparison compare, uint32_t condition, const std::string& message, uint8_t priority);

    struct ConditionData {
        ConditionData(AlarmDefinition::Condition _condition, const std::string& _message, const HashKey::EitherKey& _left = HashKey::EitherKey()) :
            condition(_condition), message(_message), leftProperty(HashKey::EitherKey()), rightProperty(HashKey::EitherKey()),
            leftValue(Value()), rightValue(Value()) {} 
        ConditionData(AlarmDefinition::Condition _condition, const std::string& _message, const HashKey::EitherKey& _left, 
            const HashKey::EitherKey& _right, const Value& _leftValue, const Value& _rightValue) : 
            condition(_condition), message(_message), leftProperty(_left), 
            rightProperty(_right), leftValue(_leftValue), rightValue(_rightValue) { }
        const AlarmDefinition::Condition condition;
        const std::string message;
        const HashKey::EitherKey leftProperty;
        const HashKey::EitherKey rightProperty;
        const Value leftValue;
        const Value rightValue;
    };
    std::vector<ConditionData> get_condition(std::shared_ptr<VariableTree> root, const HashKey::EitherKey& property);

private:
    struct LogicData {
        LogicData(const HashKey::EitherKey& _rightProperty, const Value& _constant, Comparison _compare, uint16_t _condition, const std::string& _message) :
            rightProperty(_rightProperty), constant(_constant), compare(_compare), condition(_condition), message(_message) {}
        const HashKey::EitherKey rightProperty;
        const Value constant;
        Comparison compare;
        uint32_t condition;
        std::string message;
    };
    typedef std::map<uint8_t, LogicData> PropertyMap;

    bool examine_alarm(const Value& left, const Value& right, Comparison compare);

    std::shared_mutex mapMutex;
    std::unordered_map<HashKey::EitherKey, PropertyMap, HashKey::EitherKey> property2Priority2Data;
    std::unordered_map<HashKey::EitherKey, std::unordered_set<HashKey::EitherKey, HashKey::EitherKey>, HashKey::EitherKey> rightProperty2Affected;
};

#endif