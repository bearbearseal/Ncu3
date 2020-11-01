#include "AlarmLogicConstant.h"

using namespace std;

AlarmLogicConstant::AlarmLogicConstant(AlarmDefinition::Comparison _compare, const Value& _rightValue, const std::string& _message, AlarmDefinition::Condition _condition) :
    compare(_compare), rightValue(_rightValue), message(_message), condition(_condition) {

}

AlarmLogicConstant::~AlarmLogicConstant() {

}

bool AlarmLogicConstant::condition_hit(const Value& leftValue) {
    switch(compare) {
        case AlarmDefinition::Comparison::GREATER:
            return leftValue > rightValue;
        case AlarmDefinition::Comparison::GREATER_EQUAL:
            return leftValue >= rightValue;
        case AlarmDefinition::Comparison::EQUAL:
            return leftValue == rightValue;
        case AlarmDefinition::Comparison::SMALLER_EQUAL:
            return leftValue <= rightValue;
        case AlarmDefinition::Comparison::SMALLER:
            return leftValue < rightValue;
        case AlarmDefinition::Comparison::NOT_EQUAL:
            printf("Testing %lu != %lu\n", leftValue.get_int(), rightValue.get_int());
            return leftValue != rightValue;
    }
    return false;
}

const std::string& AlarmLogicConstant::get_message() {
    return message;
}

const AlarmDefinition::Condition& AlarmLogicConstant::get_condition() {
    return condition;
}

const Value& AlarmLogicConstant::get_right_value() {
    return rightValue;
}
