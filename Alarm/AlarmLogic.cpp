#include "AlarmLogic.h"
#include <thread>

using namespace std;

AlarmDefinition::Condition nothing(AlarmDefinition::ConditionType::None, 0);
AlarmDefinition::Condition objectNotFound(AlarmDefinition::ConditionType::Error, uint32_t(AlarmDefinition::ErrorCode::ObjectDeleted));
AlarmDefinition::Condition propertyNotFound(AlarmDefinition::ConditionType::Error, uint32_t(AlarmDefinition::ErrorCode::PropertyNotFound));


AlarmLogic::AlarmLogic() {
}

AlarmLogic::~AlarmLogic() {
}

void AlarmLogic::add_logic(const HashKey::EitherKey& leftProperty, const HashKey::EitherKey& rightProperty, Comparison compare, uint32_t condition, const std::string& message, uint8_t priority) {
    LogicData logicData(rightProperty, Value(), compare, condition, message);
    {
        unique_lock<shared_mutex> lock(mapMutex);
        property2Priority2Data[leftProperty].emplace(priority, logicData);
        rightProperty2Affected[rightProperty].emplace(leftProperty);
    }
}

void AlarmLogic::add_logic(const HashKey::EitherKey& leftProperty, const Value& constant, Comparison compare, uint32_t condition, const std::string& message, uint8_t priority) {
    LogicData logicData(HashKey::EitherKey(), constant, compare, condition, message);
    {
        unique_lock<shared_mutex> lock(mapMutex);
        property2Priority2Data[leftProperty].emplace(priority, logicData);
    }
}

std::vector<AlarmLogic::ConditionData> AlarmLogic::get_condition(std::shared_ptr<VariableTree> root, const HashKey::EitherKey& property) {
    vector<AlarmLogic::ConditionData> retVal;
    shared_lock<shared_mutex> lock(mapMutex);
    //Check if this property has any AlarmLogic
    auto propertyMap = property2Priority2Data.find(property);
    if(propertyMap == property2Priority2Data.end()) {
        //retVal.push_back({nothing, ""});
        return retVal;
    }
    //Test each logic from highest priority till condition met
    for(auto i = propertyMap->second.begin(); i != propertyMap->second.end(); ++i) {
        Value leftValue, rightValue;
        //Get left value
        auto leftProperty = root->get_child(property);
        if(leftProperty == nullptr) {
            retVal.push_back({propertyNotFound, "Property not found", property});
            break;
        }
        leftValue = leftProperty->get_value();
        //Get right value
        if(!i->second.rightProperty.is_empty()){
            auto rightProperty = root->get_child(i->second.rightProperty);
            if(rightProperty == nullptr) {
                retVal.push_back({propertyNotFound, "Property not found", i->second.rightProperty});
                continue;
            }
            rightValue = rightProperty->get_value();
        }
        else {
            rightValue = i->second.constant;
        }
        //Test
        if(examine_alarm(leftValue, rightValue, i->second.compare)) {
            AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::Alarm, i->second.condition);
            retVal.push_back({newCondition, i->second.message, property, i->second.rightProperty, leftValue, rightValue});
            break;
        }
    }
    //Check affected as right property
    auto affectedLeft = rightProperty2Affected.find(property);
    if(affectedLeft == rightProperty2Affected.end()) {
        return retVal;
    }
    //For each property map that has key as the right property in at least 1 entry
    for(auto i = affectedLeft->second.begin(); i != affectedLeft->second.end(); ++i) {
        //Get the affected property
        auto subPropertyMap = property2Priority2Data.find(*i);
        //Affected property has no logic, unlikely, but check it anyway
        if(subPropertyMap == property2Priority2Data.end()) {
            continue;
        }
        //For each of the logic, only check those that has property as right property
        for(auto j = subPropertyMap->second.begin(); j != subPropertyMap->second.end(); ++j) {
            if(j->second.rightProperty == property) {
                auto leftProperty = root->get_child(*i);
                if(leftProperty == nullptr) {
                    retVal.push_back({propertyNotFound, "Property not found", *i});
                    break;
                }
                auto rightProperty = root->get_child(property);
                if(rightProperty == nullptr) {
                    retVal.push_back({propertyNotFound, "Property not found", property});
                    break;
                }
                Value leftValue = leftProperty->get_value();
                Value rightValue = rightProperty->get_value();
                if(examine_alarm(leftValue, rightValue, j->second.compare)) {
                    AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::Alarm, j->second.condition);
                    retVal.push_back({newCondition, j->second.message, *i, property, leftValue, rightValue});
                    break;
                }
            }
        }
    }
    return retVal;
}

bool AlarmLogic::examine_alarm(const Value& left, const Value& right, Comparison compare) {
    switch(compare) {
        case Comparison::GREATER:
            return left > right;
        case Comparison::GREATER_EQUAL:
            return left >= right;
        case Comparison::EQUAL:
            return left == right;
        case Comparison::SMALLER_EQUAL:
            return left <= right;
        case Comparison::SMALLER:
            return left < right;
    }
    return false;
}
