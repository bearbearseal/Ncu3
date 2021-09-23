#include "NodeAlarmManager.h"
#include <thread>

using namespace std;

NodeAlarmManager::NodeAlarmManager(std::shared_ptr<AlarmPostHandler> _alarmPostHandler) : alarmPostHandler(_alarmPostHandler)
{
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
    //Check if the equipment alreadt has node
    unordered_map<HashKey::EitherKey, size_t, HashKey::EitherKeyHash> &nodeMap = equipmentId2nodeId2IndexMap[equipmentId];
    if (!nodeMap.count(nodeId))
    {
        //The equipment doesnt exist yet, the node data would be store to new item in vector/
        entryIndex = nodeDataList.size();
        nodeMap.emplace(nodeId, entryIndex);
        nodeDataList.push_back({make_shared<VariableListener>(entryIndex, myShadow), std::map<uint16_t, uint32_t>(), AlarmDefinition::NORMAL, "Normal", equipmentId, nodeId});
    }
    else
    {
        entryIndex = nodeMap[nodeId];
    }
    nodeDataList[entryIndex].prioritzedLogicMap.insert({priority, logicId});
}

void NodeAlarmManager::set_normal_message(const HashKey::EitherKey &equipmentId, const HashKey::EitherKey &nodeId, const std::string &message)
{
    //Check if the equipment alreadt has node
    unordered_map<HashKey::EitherKey, size_t, HashKey::EitherKeyHash> &nodeMap = equipmentId2nodeId2IndexMap[equipmentId];
    if (!nodeMap.count(nodeId))
    {
        //The equipment doesnt exist yet, the node data would be store to new item in vector/
        size_t entryIndex = nodeDataList.size();
        nodeMap.emplace(nodeId, entryIndex);
        nodeDataList.push_back({make_shared<VariableListener>(entryIndex, myShadow), std::map<uint16_t, uint32_t>(), AlarmDefinition::NORMAL, message, equipmentId, nodeId});
    }
    else
    {
        size_t entryIndex = nodeMap[nodeId];
        nodeDataList[entryIndex].normalMessage = message;
    }
}

int NodeAlarmManager::attach_to_tree(std::shared_ptr<VariableTree> theTree)
{
    int failCount = 0;
    for (auto i = equipmentId2nodeId2IndexMap.begin(); i != equipmentId2nodeId2IndexMap.end(); ++i)
    {
        printf("Trying to attach %zu items.\n", i->second.size());
        auto theBranch = theTree->get_child(i->first);
        if (theBranch == nullptr || theBranch->isLeaf)
        {
            failCount += i->second.size();
            continue;
        }
        for (auto j = i->second.begin(); j != i->second.end(); ++j)
        {
            auto theLeaf = theBranch->get_child(j->first);
            if (theLeaf == nullptr || !theLeaf->isLeaf)
            {
                ++failCount;
                continue;
            }
            printf("Adding value change listener.\n");
            theLeaf->add_value_change_listener(nodeDataList[j->second].variableListener);
        }
    }
    if (failCount)
    {
        printf("NodeAlarmManager has %d alarm failed to attach.\n", failCount);
    }
    printf("Done attach.\n");
    return failCount;
}

void NodeAlarmManager::execute_check(uint32_t nodeDataIndex, const Value &theValue, std::chrono::time_point<std::chrono::system_clock> theMoment)
{
    //Not going to do error checking, if add was correct then there should have no error
    NodeData &nodeData = nodeDataList[nodeDataIndex];
    map<uint16_t, uint32_t> &theGroup = nodeData.prioritzedLogicMap;
    for (auto i = theGroup.begin(); i != theGroup.end(); ++i)
    {
        LogicData &theLogic = id2LogicMap[i->second];
        if (theLogic.verifier.verify(theValue))
        {
            if (nodeData.activeState != theLogic.state)
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
                alarmPostHandler->pickup_alarm(move(alarmMessage));
                //Update state
                nodeData.activeState = theLogic.state;
                return;
            }
        }
    }
    if (nodeData.activeState != AlarmDefinition::AlarmState::NORMAL)
    {
        AlarmDefinition::AlarmMessage alarmMessage;
        alarmMessage.activeState = AlarmDefinition::AlarmState::NORMAL;
        alarmMessage.activeValue = theValue;
        alarmMessage.code = 0;
        alarmMessage.comparison = AlarmDefinition::Comparison::EQUAL;
        alarmMessage.referenceValue = 0;
        alarmMessage.equipmentId = nodeData.equipmentId;
        alarmMessage.propertyId = nodeData.nodeId;
        alarmMessage.message = nodeData.normalMessage;
        alarmMessage.oldState = nodeData.activeState;
        alarmMessage.milliSecTime = chrono::time_point_cast<chrono::milliseconds>(theMoment).time_since_epoch().count();
        //Send message
        alarmPostHandler->pickup_alarm(move(alarmMessage));
        //Update state
        nodeData.activeState = AlarmDefinition::AlarmState::NORMAL;
    }
}

void NodeAlarmManager::VariableListener::catch_value_change_event(const std::vector<HashKey::EitherKey> &branch, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment)
{
    auto shared = master.lock();
    if (shared != nullptr)
    {
        shared->execute_check(nodeLogicGroupIndex, newValue, theMoment);
    }
}
