#include "AlarmHandler.h"

using namespace std;

AlarmHandler::AlarmHandler() {
    unordered_map<string, string> stringMap;
    unordered_map<HashKey::EitherKey, string, HashKey::EitherKey> keyMap;
    stringMap["dfsdsf"] = "dsfs";
    keyMap["dsfdf"] = "fgfgdg";
}

AlarmHandler::~AlarmHandler() {

}

void AlarmHandler::add_root_alarm_pair(std::shared_ptr<VariableTree> root, std::shared_ptr<AlarmLogic> alarmLogic, std::unordered_map<HashKey::EitherKey, uint32_t, HashKey::EitherKey>& _conditionMap) {
    PairData& pairData = pairMap[size_t(root.get())][size_t(alarmLogic.get())];
    pairData.root = root;
    pairData.alarmLogic = alarmLogic;
    pair<size_t, size_t> thePair(size_t(root.get()), size_t(alarmLogic.get()));
    pairData.valueListener = make_shared<ValueListener>(*this, thePair);//, {root.get(), alarmLogic.get()});
    for(auto entry : _conditionMap) {
        AlarmDefinition::Condition& theCondition = pairData.conditions[entry.first];
        theCondition.type = AlarmDefinition::ConditionType::Alarm;
        theCondition.code = entry.second;
    }
}

void AlarmHandler::handle_value_change(const HashKey::EitherKey& property, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment, pair<size_t, size_t> pairId) {

}
