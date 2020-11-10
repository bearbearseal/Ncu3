#include "AlarmDetector.h"
#include <iostream>

using namespace std;

AlarmDetector::AlarmDetector() {
}

AlarmDetector::~AlarmDetector() {
    //No thread, no need to stop anything
}

void AlarmDetector::set_listener(std::shared_ptr<AlarmListener> _listener) {
    listener = _listener;
}

void AlarmDetector::add_root_alarm_pair(
    const HashKey::EitherKey& equipmentId, shared_ptr<VariableTree> root, shared_ptr<AlarmLogic> alarmLogic, 
    const unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey>& _conditionMap
) {
    PairData& pairData = pairMap[size_t(root.get())][size_t(alarmLogic.get())];
    pairData.equipment = equipmentId;
    pairData.root = root;
    pairData.alarmLogic = alarmLogic;
    pair<size_t, size_t> thePair(size_t(root.get()), size_t(alarmLogic.get()));
    pairData.valueListener = make_shared<ValueListener>(*this, thePair);//, {root.get(), alarmLogic.get()});
    for(auto entry : _conditionMap) {
        lock_guard<mutex> lock(pairData.conditionMutex);
        pairData.conditions[entry.first] = entry.second;
        cout<<"Equipment: "<<equipmentId.to_string()<<endl;
        cout<<"Property: "<<entry.first.to_string()<<endl;
        cout<<"Condition:"<<entry.second.to_string()<<endl;
        /*
        AlarmDefinition::Condition& theCondition = pairData.conditions[entry.first];
        theCondition.type = AlarmDefinition::ConditionType::Alarm;
        theCondition.code = entry.second;
        */
    }
    root->add_value_change_listener(pairData.valueListener);
}

void AlarmDetector::handle_value_change(const HashKey::EitherKey& property, pair<size_t, size_t> pairId) {
    auto sharedListener = listener.lock();
    if(sharedListener == nullptr) {
        return;
    }
    auto i = pairMap.find(pairId.first);
    if(i == pairMap.end()) {
        return;
    }
    auto j = i->second.find(pairId.second);
    if(j == i->second.end()) {
        return;
    }
    PairData& pairData = j->second;
    auto sharedRoot = pairData.root.lock();
    if(sharedRoot == nullptr) {
        return;
    }
    auto sharedLogic = pairData.alarmLogic.lock();
    if(sharedLogic == nullptr) {
        return;
    }
    chrono::time_point<chrono::system_clock> theMoment = chrono::system_clock::now();
    auto conditionList = sharedLogic->get_condition(sharedRoot, property);
    for(auto singleCondition : conditionList) {
        //compare if condition has changed
        lock_guard<mutex> lock(pairData.conditionMutex);
        auto previousCondition = pairData.conditions.find(singleCondition.leftProperty);
        if(previousCondition == pairData.conditions.end()) {
            //No previous condition, then it's an alarm
            sharedListener->catch_alarm({pairData.equipment, singleCondition.leftProperty, singleCondition.leftValue,
                singleCondition.rightProperty, singleCondition.rightValue, singleCondition.message, theMoment, 
                singleCondition.condition});
            pairData.conditions[singleCondition.leftProperty] = singleCondition.condition;
            cout<<singleCondition.leftProperty.to_string()<<" condition is now "<<singleCondition.condition.to_string()<<endl;
        }
        else if(previousCondition->second != singleCondition.condition) {
            //Condition changed, it's an alarm
            sharedListener->catch_alarm({pairData.equipment, singleCondition.leftProperty, singleCondition.leftValue,
                singleCondition.rightProperty, singleCondition.rightValue, singleCondition.message, theMoment,
                singleCondition.condition});
            pairData.conditions[singleCondition.leftProperty] = singleCondition.condition;
            cout<<singleCondition.leftProperty.to_string()<<" condition is now "<<singleCondition.condition.to_string()<<endl;
        }
        else {
            cout<<singleCondition.leftProperty.to_string()<<" condition remains unchanged.\n";
            cout<<"Old condition: "<<previousCondition->second.to_string()<<endl;
            cout<<"New condition: "<<singleCondition.condition.to_string()<<endl;
        }
    }
}

//To add delay, put the change to memory, check after defined period of time.
//If any changes in between that period of time, then extend the period of time.