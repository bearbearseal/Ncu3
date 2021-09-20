#include <chrono>
#include "AlarmDetector.h"

using namespace std;

AlarmDetector::AlarmDetector(const HashKey::EitherKey& equipmentId, const HashKey::EitherKey& propertyId, std::weak_ptr<Listener> _listener) : myId({equipmentId, propertyId}) {
    listener = _listener;
}

AlarmDetector::AlarmDetector(const AlarmDefinition::PointId& _myId, std::weak_ptr<Listener> _listener) : myId(_myId) {
    listener = _listener;
}

AlarmDetector::~AlarmDetector() {

}

void AlarmDetector::add_logic(uint8_t priority, const AlarmDefinition::AlarmLogicConstant& logicData) {
    logicMap.emplace(priority, AlarmLogicConstant(logicData.compare, logicData.rightValue, logicData.message, logicData.condition));
    //printf("Total logic in map: %lu\n", logicMap.size());
}

void AlarmDetector::catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
    printf("Checking alarm.\n");
    if(newValue.is_empty()) {
        return;
    }
    AlarmMessage alarmMessage;
    alarmMessage.pointId = myId;
    alarmMessage.value = newValue;
    auto milliSec = chrono::time_point_cast<chrono::milliseconds>(theMoment);
    auto epoch = milliSec.time_since_epoch();
    alarmMessage.milliSecTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    auto shared = listener.lock();
    if(shared == nullptr) {
        printf("No listener.\n");
        return;
    }
    for(auto& entry : logicMap) {
        if(entry.second.condition_hit(newValue)) {
            AlarmMessage alarmMessage;
            alarmMessage.pointId = myId;
            alarmMessage.value = newValue;
            //printf("Value: %s\n", alarmMessage.value.to_string().c_str());
            alarmMessage.rightValue = entry.second.get_right_value();
            auto milliSec = chrono::time_point_cast<chrono::milliseconds>(theMoment);
            auto epoch = milliSec.time_since_epoch();
            alarmMessage.milliSecTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
            alarmMessage.condition = entry.second.get_condition();
            alarmMessage.message = entry.second.get_message();
            //printf("Found alarm.\n");
            shared->catch_alarm(alarmMessage);
            return;
        }
    }
    alarmMessage.condition = {AlarmDefinition::ConditionType::None, 0};
    shared->catch_alarm(alarmMessage);
    //printf("No alarm.\n");
    return;
}
