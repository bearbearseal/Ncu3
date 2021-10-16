#ifndef _AlarmManager_H_
#define _AlarmManager_H_
#include "AlarmDefinition.h"
#include "AlarmLogicGroup.h"
#include "../Storage/ConfigStorage.h"
#include "../VariableTree/VariableTree.h"

//Hold multiple AlarmLogicGroup
//

class AlarmManager
{
    friend class Shadow;

public:
    class AlarmListener
    {
    public:
        AlarmListener() {}
        virtual ~AlarmListener() {}
        virtual void catch_alarm(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                                 const Value &refValue, AlarmDefinition::Comparison compare, const std::chrono::time_point<std::chrono::system_clock> theMoment) = 0;
    };
    AlarmManager(ConfigStorage &storage, std::shared_ptr<VariableTree> target, std::unique_ptr<AlarmListener> &alarmPoster);
    virtual ~AlarmManager();

private:
    class Shadow
    {
    public:
        Shadow(AlarmManager &_me) : me(_me) {}
        void handle_value_change(uint64_t pointId, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { me.handle_value_change(pointId, newValue, theMoment); }

    private:
        AlarmManager &me;
    };

    class AlarmActor : public VariableTree::ValueChangeListener
    {
    public:
        AlarmActor(std::weak_ptr<Shadow> _master, uint64_t _equipmentPointId) : master(_master), pointId(_equipmentPointId) {}
        void catch_value_change_event(const std::vector<HashKey::EitherKey> &branch, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

    private:
        std::weak_ptr<Shadow> master;
        const uint64_t pointId;
    };

    void handle_value_change(uint64_t pointId, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

    std::shared_ptr<Shadow> myShadow;
    std::unique_ptr<AlarmListener> alarmPoster;
    std::unordered_map<uint32_t, AlarmLogicGroup> logicGroupMap;
    struct ValueCatcherData
    {
        uint32_t alarmGroupId;
        std::shared_ptr<AlarmActor> alarmActor;
    };
    std::map<uint64_t, ValueCatcherData> pointId2LogicGroupMap;
};

#endif