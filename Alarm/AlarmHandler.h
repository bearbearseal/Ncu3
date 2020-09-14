#ifndef _AlarmHandler_H_
#define _AlarmHandler_H_
#include <memory>
#include <unordered_map>
#include "AlarmLogic.h"
#include "../Basic/HashKey.h"
#include "AlarmDefinition.h"
#include "AlarmListener.h"
#include "../VariableTree/VariableTree.h"

class AlarmHandler {
    friend class ValueListener;
public:
    class ValueListener : public VariableTree::ValueChangeListener {
    public:
        ValueListener(AlarmHandler& _master, const std::pair<size_t, size_t>& _pairId) : master(_master), pairId(_pairId) {}
        virtual ~ValueListener() {}
		virtual void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
            master.handle_value_change(branch.front(), newValue, theMoment, pairId);
        }

    private:
        AlarmHandler& master;
        const std::pair<size_t, size_t> pairId;
    };

    AlarmHandler();
    virtual ~AlarmHandler();
    void add_root_alarm_pair(std::shared_ptr<VariableTree> root, std::shared_ptr<AlarmLogic> alarmLogic, std::unordered_map<HashKey::EitherKey, uint32_t, HashKey::EitherKey>& _conditionMap);

private:
    void handle_value_change(const HashKey::EitherKey& property, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment, std::pair<size_t, size_t> pairId);

    struct PairData {
        std::weak_ptr<VariableTree> root;
        std::weak_ptr<AlarmLogic> alarmLogic;
        std::shared_ptr<ValueListener> valueListener;
        std::unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey> conditions;
    };
    //root id, logic id
    std::unordered_map<size_t, std::unordered_map<size_t, PairData>> pairMap;
};

#endif