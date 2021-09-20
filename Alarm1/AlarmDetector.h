#pragma once
#include <memory>
#include <map>
#include "AlarmLogicVerifier.h"
#include "AlarmDefinition.h"
//#include "AlarmListener.h"
#include "../VariableTree/VariableTree.h"

/*
Listen for value change of a node,
Do the logic processing whenever caught a value change
*/

class AlarmDetector : public VariableTree::ValueChangeListener
{
public:
    struct AlarmMessage {
        AlarmDefinition::PointId pointId;
        Value value;
        Value rightValue;
        AlarmDefinition::Condition condition;
        uint64_t milliSecTime;
        std::string message;
    };
    class Listener
    {
    public:
        Listener(){}
        virtual ~Listener();
        virtual void catch_alarm(const AlarmMessage &alarmMessage) = 0;
    };
    AlarmDetector(const HashKey::EitherKey &equipmentId, const HashKey::EitherKey &propertyId, std::weak_ptr<Listener> listener);
    AlarmDetector(const AlarmDefinition::PointId &_myId, std::weak_ptr<Listener> listener);
    ~AlarmDetector();

    //void set_alarm_listener(std::shared_ptr<Listener> _listener);
    void add_logic(uint8_t priority, const AlarmDefinition::AlarmLogicConstant &logicData);
    void catch_value_change_event(const std::vector<HashKey::EitherKey> &branch, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

private:
    const AlarmDefinition::PointId myId;
    std::weak_ptr<Listener> listener;
    std::map<uint8_t, AlarmLogicConstant> logicMap;
};