#include "NodeAlarmManager.h"
#include <thread>

using namespace std;

NodeAlarmManager::NodeAlarmManager(std::shared_ptr<AlarmListener> _alarmListener)
{
    alarmListener = _alarmListener;
    myShadow = make_shared<Shadow>(this);
}

NodeAlarmManager::~NodeAlarmManager()
{
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while (weak.lock())
    {
        this_thread::yield();
    }
}

void NodeAlarmManager::add_alarm_logic(uint32_t logicId, AlarmDefinition::Comparison comparison, const Value &referenceValue, const std::string &message, AlarmDefinition::AlarmState state, uint16_t code)
{
    id2LogicMap.insert({logicId, {message, state, code, AlarmVerifier(comparison, referenceValue)}});
}

void NodeAlarmManager::set_node_logic(const HashKey::EitherKey &equipmentId, const HashKey::EitherKey &nodeId, uint32_t logicId, uint16_t priority)
{
    size_t entryIndex;
    unordered_map<HashKey::EitherKey, size_t, HashKey::EitherKeyHash> &nodeMap = equipmentId2nodeId2IndexMap[equipmentId];
    if (!nodeMap.count(nodeId))
    {
        entryIndex = nodeDataList.size();
        nodeDataList.push_back({make_shared<VariableListener>(entryIndex, myShadow), std::map<uint16_t, uint32_t>(), AlarmDefinition::Normal, equipmentId, nodeId});
    }
    else
    {
        entryIndex = nodeMap[nodeId];
    }
    nodeDataList[entryIndex].prioritzedLogicMap.insert({priority, logicId});
}

int NodeAlarmManager::attach_to_tree(std::shared_ptr<VariableTree> theTree)
{
    int failCount = 0;
    for (auto i = equipmentId2nodeId2IndexMap.begin(); i != equipmentId2nodeId2IndexMap.end(); ++i)
    {
        auto theBranch = theTree->get_child(i->first);
        if(theBranch == nullptr || theBranch->isLeaf)
        {
            failCount += i->second.size();
            continue;
        }
        for(auto j = i->second.begin(); j != i->second.end(); ++j)
        {
            auto theLeaf = theBranch->get_child(j->first);
            if(theLeaf == nullptr || !theLeaf->isLeaf)
            {
                ++failCount;
                continue;
            }
            theLeaf->add_value_change_listener(nodeDataList[j->second].variableListener);
        }
    }
    if(failCount)
    {
        printf("NodeAlarmManager has %d alarm failed to attach.\n");
    }
}

void NodeAlarmManager::execute_check(uint32_t nodeDataIndex, const Value &theValue, std::chrono::time_point<std::chrono::system_clock> theMoment)
{
    //Not going to do error checking, if add was correct then there should have no error
    NodeData& nodeData = nodeDataList[nodeDataIndex];
    map<uint16_t, uint32_t>& theGroup = nodeData.prioritzedLogicMap;
    for (auto i = theGroup.begin(); i != theGroup.end(); ++i)
    {
        LogicData& theLogic = id2LogicMap[i->second];
        if (theLogic.verifier.verify(theValue))
        {
            if(nodeData.activeState != theLogic.state)
            {
                //change of state
                //Create message
                AlarmDefinition::AlarmMessage alarmMessage;
                alarmMessage.activeState = theLogic.state;
                alarmMessage.activeValue = theValue;
                alarmMessage.code = theLogic.code;
                alarmMessage.comparison = theLogic.verifier.get_compare();
                alarmMessage.referenceValue = theLogic.verifier.get_value();
                alarmMessage.equipmentId = nodeData.equipmentId;
                alarmMessage.propertyId = nodeData.nodeId;
                alarmMessage.message = theLogic.alarmMessage;
                alarmMessage.oldState = nodeData.activeState;
                alarmMessage.milliSecTime = chrono::time_point_cast<chrono::milliseconds>(theMoment).time_since_epoch().count();
                //Send message
                alarmListener->catch_alarm(alarmMessage);
                //Update state
                nodeData.activeState = theLogic.state;
            }
        }
    }
}
