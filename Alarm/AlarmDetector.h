#pragma once
#include <memory>
#include <map>
#include "AlarmLogicConstant.h"
#include "AlarmDefinition.h"
#include "AlarmListener.h"
#include "../VariableTree/VariableTree.h"

class AlarmDetector : public VariableTree::ValueChangeListener {
public:
    AlarmDetector(const HashKey::EitherKey& equipmentId, const HashKey::EitherKey& propertyId);
    AlarmDetector(const AlarmDefinition::PointId& _myId);
    ~AlarmDetector();

    void set_alarm_listener(std::shared_ptr<AlarmListener> _listener);
    void add_logic(uint8_t priority, const AlarmDefinition::AlarmLogicConstant& logicData);
	void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

private:
    const AlarmDefinition::PointId myId;
    std::weak_ptr<AlarmListener> listener;
    std::map<uint8_t, AlarmLogicConstant> logicMap;
};