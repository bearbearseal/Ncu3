#include "AlarmManager.h"
#include <thread>

using namespace std;

inline uint64_t meld_equipment_point_id(uint32_t equipmentId, uint32_t pointId)
{
    uint64_t retVal = equipmentId;
    retVal <<= 32;
    retVal += pointId;
    return retVal;
}

inline pair<uint32_t, uint32_t> split_equipment_point_id(uint64_t combinedId)
{
    return {static_cast<uint32_t>(combinedId >> 32), static_cast<uint32_t>(combinedId)};
}

AlarmManager::AlarmManager(ConfigStorage &storage, std::shared_ptr<VariableTree> target, std::unique_ptr<AlarmListener> &_alarmPoster)
{
    myShadow = make_shared<Shadow>(*this);
    //Create alarm logic group
    auto alarmLogicData = storage.get_alarm_logic();
    for (size_t i = 0; i < alarmLogicData.size(); ++i)
    {
        vector<AlarmLogicGroup::AlarmData> alarmDataList;
        alarmDataList.resize(alarmLogicData[i].logicData.size());
        for (size_t j = 0; j < alarmLogicData[i].logicData.size(); ++j)
        {
            alarmDataList[j].compare = static_cast<AlarmDefinition::Comparison>(alarmLogicData[i].logicData[j].compare);
            alarmDataList[j].refValue = alarmLogicData[i].logicData[j].refValue;
            alarmDataList[j].state = static_cast<AlarmDefinition::AlarmState>(alarmLogicData[i].logicData[j].state);
        }
        logicGroupMap[i] = AlarmLogicGroup(alarmDataList);
    }
    //Load the pairing information
    auto alarmPairingInfo = storage.get_alarm_point_pair();
    for (size_t i = 0; i < alarmPairingInfo.size(); ++i)
    {
        auto equipment = target->get_child(alarmPairingInfo[i].deviceId);
        if (equipment != nullptr)
        {
            auto device = equipment->get_child(alarmPairingInfo[i].pointId);
            if (device != nullptr)
            {
                uint64_t equipmentPoint = meld_equipment_point_id(alarmPairingInfo[i].deviceId, alarmPairingInfo[i].pointId);
                ValueCatcherData &entry = pointId2LogicGroupMap[equipmentPoint];
                entry.alarmGroupId = alarmPairingInfo[i].logicGroupId;
                entry.alarmActor = make_shared<AlarmActor>(myShadow, equipmentPoint);
                device->add_value_change_listener(entry.alarmActor);
            }
        }
    }
    alarmPoster = move(_alarmPoster);
}

AlarmManager::~AlarmManager()
{
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while (weak.lock())
    {
        this_thread::yield();
    }
}

void AlarmManager::handle_value_change(uint64_t pointId, const Value &newValue, chrono::time_point<chrono::system_clock> theMoment)
{
    //call alarm logic group handler to handle
    ValueCatcherData &valueCatcherData = pointId2LogicGroupMap[pointId];
    uint32_t alarmGroupId = valueCatcherData.alarmGroupId;
    AlarmLogicGroup &logicGroup = logicGroupMap[alarmGroupId];
    auto result = logicGroup.check_alarm(newValue);
    if (result.has_value())
    {
        //Report it to listener;
        auto equipmentPointId = split_equipment_point_id(pointId);
        alarmPoster->catch_alarm(equipmentPointId.first, equipmentPointId.second, newValue, result->state, result->refValue, result->compare, theMoment);
    }
}

void AlarmManager::AlarmActor::catch_value_change_event(const vector<HashKey::EitherKey> &branch, const Value &newValue, chrono::time_point<chrono::system_clock> theMoment)
{
    auto shared = master.lock();
    if(shared != nullptr)
    {
        shared->handle_value_change(pointId, newValue, theMoment);
    }
}
