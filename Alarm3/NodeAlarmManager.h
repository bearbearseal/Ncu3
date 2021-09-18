#ifndef _AlarmManager_H_
#define _AlarmManager_H_
#include "../VariableTree/VariableTree.h"
#include "AlarmDefinition.h"
#include "AlarmVerifier.h"

class NodeAlarmManager
{
    friend class Shadow;

private:
    class Shadow
    {
    public:
        Shadow(NodeAlarmManager *_me) : me(_me) {}
        void execute_check(uint32_t nodeLogicGroupIndex, const Value &theValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { me->execute_check(nodeLogicGroupIndex, theValue, theMoment); }

    private:
        NodeAlarmManager *me;
    };
    class VariableListener : public VariableTree::ValueChangeListener
    {
    public:
        VariableListener(size_t _nodeLogicGroupIndex, std::weak_ptr<Shadow> _master) : nodeLogicGroupIndex(_nodeLogicGroupIndex), master(_master) {}
        void catch_value_change_event(const std::vector<HashKey::EitherKey> &branch, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

    private:
        size_t nodeLogicGroupIndex;
        std::weak_ptr<Shadow> master;
    };
    struct LogicData
    {
        std::string alarmMessage;
        AlarmDefinition::AlarmState state;
        uint16_t code;
        AlarmVerifier verifier;
    };
    struct NodeData
    {
        std::shared_ptr<VariableListener> variableListener;
        std::map<uint16_t, uint32_t> prioritzedLogicMap;
        AlarmDefinition::AlarmState activeState;
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey nodeId;
    };

public:
    class AlarmListener
    {
    public:
        AlarmListener() {}
        virtual ~AlarmListener() {}
        virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarm) { printf("Please override report_alarm method.\n of AlarmReporter.\n"); }
    };

    //alarmReporter shall not be deleted b4 this class instance or weak_ptr shall be taken.
    NodeAlarmManager(std::shared_ptr<AlarmListener> _alarmListener);
    virtual ~NodeAlarmManager();
    void add_alarm_logic(uint32_t logicId, AlarmDefinition::Comparison comparison, const Value &referenceValue, const std::string &message, AlarmDefinition::AlarmState state, uint16_t code);
    void set_node_logic(const HashKey::EitherKey &equipmentId, const HashKey::EitherKey &nodeId, uint32_t logicId, uint16_t priority);
    int attach_to_tree(std::shared_ptr<VariableTree> theTree);

private:
    void execute_check(uint32_t nodeDataIndex, const Value &theValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

    std::shared_ptr<AlarmListener> alarmListener;
    std::shared_ptr<Shadow> myShadow;
    //integer hashkey speed is good
    //Collection of logic, could be referred with id
    std::unordered_map<uint32_t, LogicData> id2LogicMap;
    //Intermediate data structure, referred only when at preparation, not being referred at run time.
    std::unordered_map<HashKey::EitherKey, std::unordered_map<HashKey::EitherKey, size_t, HashKey::EitherKeyHash>, HashKey::EitherKeyHash> equipmentId2nodeId2IndexMap;
    std::vector<NodeData> nodeDataList;
};

#endif