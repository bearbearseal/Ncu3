#include "AlarmLogic_back.h"
#include <thread>

using namespace std;

AlarmLogic::AlarmLogic(std::shared_ptr<VariableTree> root) {
    subject = root;
    myShadow = make_shared<Shadow>(*this);
    root->add_value_change_listener(myShadow);
}

AlarmLogic::~AlarmLogic() {
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while(weak.lock()) {
        this_thread::sleep_for(10ms);
    }
}

void AlarmLogic::set_listener(std::shared_ptr<AlarmListener> _listener) {
    listener = _listener;
}

void AlarmLogic::set_condition(const HashKey::EitherKey& leftProperty, uint32_t condition) {
    lock_guard<mutex> lock(mapMutex);
    auto i = property2Priority2Data.find(leftProperty);
    if(i != property2Priority2Data.end()) {
        i->second.activeCondition = condition;
    }
}

void AlarmLogic::add_logic(const HashKey::EitherKey& leftProperty, const HashKey::EitherKey& rightProperty, Comparison compare, uint32_t condition, uint8_t priority) {
    LogicData logicData(rightProperty, Value(), compare, condition);
    {
        lock_guard<mutex> lock(mapMutex);
        property2Priority2Data[leftProperty].logicData.emplace(priority, logicData);
        rightProperty2Affected[rightProperty].emplace(leftProperty);
    }
}

void AlarmLogic::add_logic(const HashKey::EitherKey& leftProperty, const Value& constant, Comparison compare, uint32_t condition, uint8_t priority) {
    LogicData logicData(HashKey::EitherKey(), constant, compare, condition);
    {
        lock_guard<mutex> lock(mapMutex);
        property2Priority2Data[leftProperty].logicData.emplace(priority, logicData);
    }
}

AlarmLogic::LogicData::LogicData(const HashKey::EitherKey& _rightProperty, const Value& _constant, Comparison _compare, uint16_t _condition) 
 : rightProperty(_rightProperty), constant(_constant), compare(_compare), condition(_condition) {

}

void AlarmLogic::catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
    //Lock the subject
    auto sharedSubject = subject.lock();
    if(sharedSubject == nullptr) {
        return;
    }
    //Lock listener
    auto sharedListener = listener.lock();
    if(sharedListener == nullptr) {
        return;
    }
    HashKey::EitherKey key = *(branch.begin());
    //check normal map
    auto propertyMap = property2Priority2Data.find(key);
    if(propertyMap == property2Priority2Data.end()) {
        return;
    }
    for(auto i = propertyMap->second.logicData.begin(); i != propertyMap->second.logicData.end(); ++i) {
        Value leftValue, rightValue;
        auto leftProperty = sharedSubject->get_child(key);
        if(leftProperty == nullptr) {
            //Generate property not found error
            return;
        }
        leftValue = leftProperty->get_value();
        if(i->second.constant.is_empty()) {
            auto rightProperty = sharedSubject->get_child(i->second.rightProperty);
            if(rightProperty == nullptr) {
                //Generate property not found error
                return;
            }
            rightValue = rightProperty->get_value();
        }
        else {
            rightValue = i->second.constant;
        }
        if(examine_alarm(leftValue, rightValue, i->second.compare)) {
            if(propertyMap->second.activeCondition != i->second.condition) {
                //Generate alarm;
                propertyMap->second.activeCondition = i->second.condition;
                break;
            }
        }
    }
    //Check affected map
    auto affectedProperty = rightProperty2Affected.find(key);
    if(affectedProperty == rightProperty2Affected.end()) {
        return;
    }
    //For each property map that has key as the right property in at least 1 entry
    for(auto i = affectedProperty->second.begin(); i != affectedProperty->second.end(); ++i) {
        auto subPropertyMap = property2Priority2Data.find(*i);
        if(subPropertyMap == property2Priority2Data.end()) {
            break;
        }
        for(auto j = subPropertyMap->second.logicData.begin(); j != subPropertyMap->second.logicData.end(); ++j) {
            if(j->second.rightProperty == key) {
                auto leftProperty = sharedSubject->get_child(*i);
                if(leftProperty == nullptr) {
                    //Generate property not found error
                    return;
                }
                auto rightProperty = sharedSubject->get_child(key);
                if(rightProperty == nullptr) {
                    //Generate property not found error
                    return;
                }
                Value leftValue = leftProperty->get_value();
                Value rightValue = rightProperty->get_value();
                if(examine_alarm(leftValue, rightValue, j->second.compare)) {
                    if(propertyMap->second.activeCondition != j->second.condition) {
                        //Generate alarm;
                        propertyMap->second.activeCondition = j->second.condition;
                        break;
                    }
                }
                break;
            }
        }
    }
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
