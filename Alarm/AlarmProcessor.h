#ifndef _AlarmProcessor_H_
#define _AlarmProcessor_H_
#include "AlarmDefinition.h"
#include "AlarmLogicGroup.h"
#include "../Storage/ConfigStorage.h"
#include "../VariableTree/VariableTree.h"

//Hold multiple AlarmLogicGroup
//Process value change to alarm state

class AlarmProcessor
{
    friend class Shadow;

public:
    class ProcessedStateListener
    {
    public:
        ProcessedStateListener() {}
        virtual ~ProcessedStateListener() {}
        virtual void catch_alarm_state(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                                 const Value &refValue, AlarmDefinition::Comparison compare, const std::chrono::time_point<std::chrono::system_clock> theMoment) = 0;
    };
    AlarmProcessor(ConfigStorage &storage, std::shared_ptr<VariableTree> target, std::shared_ptr<ProcessedStateListener> processedStateListener);
    virtual ~AlarmProcessor();

private:
    class Shadow
    {
    public:
        Shadow(AlarmProcessor &_me) : me(_me) {}
        void handle_value_change(uint64_t pointId, const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { me.handle_value_change(pointId, newValue, theMoment); }

    private:
        AlarmProcessor &me;
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
    std::shared_ptr<ProcessedStateListener> stateProcessor;
    std::unordered_map<uint32_t, AlarmLogicGroup> logicGroupMap;
    struct ValueCatcherData
    {
        uint32_t alarmGroupId;
        std::shared_ptr<AlarmActor> alarmActor;
    };
    std::map<uint64_t, ValueCatcherData> pointId2LogicGroupMap;
};

#endif