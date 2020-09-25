#include "AlarmLogic.h"
#include "../../MyLib/UdpSocket/UdpSocket.h"
#include <thread>
#include <iostream>

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
    //cout<<"Getting condition for property "<<property.to_string()<<endl;
    vector<AlarmLogic::ConditionData> retVal;
    shared_lock<shared_mutex> lock(mapMutex);
    //Check if this property has any AlarmLogic
    auto propertyMap = property2Priority2Data.find(property);
    if(propertyMap == property2Priority2Data.end()) {
        cout<<"No logic for this property.\n";
        //retVal.push_back({nothing, ""});
        //return retVal;
    }
    else {
        auto leftProperty = root->get_child(property);
        if(leftProperty == nullptr) {
            //cout<<"Left property not found "<<property.to_string()<<endl;
            retVal.push_back({propertyNotFound, "Property not found", property});
        }
        else {
            bool hitCondition = false;
            //Test each logic from highest priority till condition met
            auto leftValue = leftProperty->get_value();
            for(auto i = propertyMap->second.rbegin(); i != propertyMap->second.rend(); ++i) {
                //cout<<"Checking logic with priority "<<size_t(i->first)<<endl;
                Value rightValue;
                //Get left value
                //Get right value
                if(!i->second.rightProperty.is_empty()){
                    auto rightProperty = root->get_child(i->second.rightProperty);
                    if(rightProperty == nullptr) {
                        //cout<<"Right property not found "<<i->second.rightProperty.to_string()<<endl;
                        retVal.push_back({propertyNotFound, "Property not found", i->second.rightProperty});
                        continue;
                    }
                    rightValue = rightProperty->get_value();
                }
                else {
                    rightValue = i->second.constant;
                }
                //If either value is empty, skip
                if(leftValue.is_empty() || rightValue.is_empty()) {
                    continue;
                }
                //Test
                if(examine_alarm(leftValue, rightValue, i->second.compare)) {
                    //cout<<"Condition hit!\n";
                    AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::Alarm, i->second.condition);
                    retVal.push_back({newCondition, i->second.message, property, i->second.rightProperty, leftValue, rightValue});
                    hitCondition = true;
                    break;
                }
            }
            //If none of the conditions are met
            if(!hitCondition) {
                //cout<<"No condition hit.\n";
                AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::None, 0);
                retVal.push_back({newCondition, "", property, HashKey::EitherKey(), leftValue, Value()});
            }
        }
    }
    //Check affected as right property
    //cout<<"Checking right.\n";
    auto affectedLeft = rightProperty2Affected.find(property);
    if(affectedLeft == rightProperty2Affected.end()) {
        //cout<<"No affecting any logic as right property.\n";
        return retVal;
    }
    //For each property map that has key as the right property in at least 1 entry
    for(auto i = affectedLeft->second.begin(); i != affectedLeft->second.end(); ++i) {
        //Get the affected property
        auto subPropertyMap = property2Priority2Data.find(*i);
        //Affected property has no logic, unlikely, but check it anyway
        if(subPropertyMap == property2Priority2Data.end()) {
            //cout<<"Affected logic has no right property??\n";
            continue;
        }
        cout<<"Affected left property "<<i->to_string()<<endl;
        auto leftProperty = root->get_child(*i);
        if(leftProperty == nullptr) {
            retVal.push_back({propertyNotFound, "Property not found", *i});
        }
        else {
            bool hitCondition = false;
            Value leftValue = leftProperty->get_value();
            //For each of the logic, checks everything
            for(auto j = subPropertyMap->second.rbegin(); j != subPropertyMap->second.rend(); ++j) {
                auto rightProperty = root->get_child(property);
                if(rightProperty == nullptr) {
                    retVal.push_back({propertyNotFound, "Property not found", property});
                    break;
                }
                Value rightValue = rightProperty->get_value();
                //If either value is empty, skip
                if(leftValue.is_empty() || rightValue.is_empty()) {
                    continue;
                }
                if(examine_alarm(leftValue, rightValue, j->second.compare)) {
                    //cout<<"Right property logic condition hit!\n";
                    AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::Alarm, j->second.condition);
                    retVal.push_back({newCondition, j->second.message, *i, property, leftValue, rightValue});
                    hitCondition = true;
                    break;
                }
            }
            //If none of the conditions are met
            if(!hitCondition) {
                cout<<"Right property no condition hit.\n";
                AlarmDefinition::Condition newCondition(AlarmDefinition::ConditionType::None, 0);
                retVal.push_back({newCondition, "", *i, HashKey::EitherKey(), leftValue, Value()});
            }
        }
    }
    //cout<<"Returning with "<<retVal.size()<<" conditions.\n";
    return retVal;
}

bool AlarmLogic::examine_alarm(const Value& left, const Value& right, Comparison compare) {
    switch(compare) {
        case Comparison::GREATER:
            //cout<<"Checking "<<left.to_string()<<" > "<<right.to_string()<<endl;
            return left > right;
        case Comparison::GREATER_EQUAL:
            //cout<<"Checking "<<left.to_string()<<" >= "<<right.to_string()<<endl;
            return left >= right;
        case Comparison::EQUAL:
            //cout<<"Checking "<<left.to_string()<<" == "<<right.to_string()<<endl;
            return left == right;
        case Comparison::SMALLER_EQUAL:
            //cout<<"Checking "<<left.to_string()<<" <= "<<right.to_string()<<endl;
            return left <= right;
        case Comparison::SMALLER:
            //cout<<"Checking "<<left.to_string()<<" < "<<right.to_string()<<endl;
            return left < right;
        case Comparison::NOT_EQUAL:
            //cout<<"Checking "<<left.to_string()<<" != "<<right.to_string()<<endl;
            return left != right;
    }
    return false;
}
