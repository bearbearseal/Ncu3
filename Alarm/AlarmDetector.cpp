#include <chrono>
#include "AlarmDetector.h"

using namespace std;

AlarmDetector::AlarmDetector(const HashKey::EitherKey& equipmentId, const HashKey::EitherKey& propertyId) : myId({equipmentId, propertyId}) {

}

AlarmDetector::AlarmDetector(const AlarmDefinition::PointId& _myId) : myId(_myId) {

}

AlarmDetector::~AlarmDetector() {

}

void AlarmDetector::set_alarm_listener(std::shared_ptr<AlarmListener> _listener) {
    listener = _listener;
}

void AlarmDetector::add_logic(uint8_t priority, const AlarmDefinition::AlarmLogicConstant& logicData) {
    logicMap.emplace(priority, AlarmLogicConstant(logicData.compare, logicData.rightValue, logicData.message, logicData.condition));
}

void AlarmDetector::catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
    AlarmDefinition::AlarmMessage alarmMessage;
    alarmMessage.pointId = myId;
    alarmMessage.value = newValue;
    auto milliSec = chrono::time_point_cast<chrono::milliseconds>(theMoment);
    auto epoch = milliSec.time_since_epoch();
    alarmMessage.milliSecTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    auto shared = listener.lock();
    if(shared == nullptr) {
        return;
    }
    for(auto& entry : logicMap) {
        if(entry.second.condition_hit(newValue)) {
            AlarmDefinition::AlarmMessage alarmMessage;
            alarmMessage.pointId = myId;
            alarmMessage.value = newValue;
            alarmMessage.rightValue = entry.second.get_right_value();
            auto milliSec = chrono::time_point_cast<chrono::milliseconds>(theMoment);
            auto epoch = milliSec.time_since_epoch();
            alarmMessage.milliSecTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
            alarmMessage.condition = entry.second.get_condition();
            alarmMessage.message = entry.second.get_message();
            shared->catch_alarm(alarmMessage);
            return;
        }
    }
    alarmMessage.condition = {AlarmDefinition::ConditionType::None, 0};
    shared->catch_alarm(alarmMessage);
    return;
}
