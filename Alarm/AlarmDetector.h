#ifndef _AlarmDetector_H_
#define _AlarmDetector_H_
#include <memory>
#include <unordered_map>
#include "AlarmLogic.h"
#include "../../MyLib/Basic/HashKey.h"
#include "AlarmDefinition.h"
#include "AlarmListener.h"
#include "../VariableTree/VariableTree.h"

//Doesnt support remove of pair map yet, which would need more mutex
class AlarmDetector {
    friend class ValueListener;
public:
    class ValueListener : public VariableTree::ValueChangeListener {
    public:
        ValueListener(AlarmDetector& _master, const std::pair<size_t, size_t>& _pairId) : master(_master), pairId(_pairId) {}
        virtual ~ValueListener() {}
		virtual void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) {
            master.handle_value_change(branch.front(), pairId);
        }

    private:
        AlarmDetector& master;
        const std::pair<size_t, size_t> pairId;
    };

    AlarmDetector();
    virtual ~AlarmDetector();
    void set_listener(std::shared_ptr<AlarmListener> _listener);
    void add_root_alarm_pair(const HashKey::EitherKey& equipment, std::shared_ptr<VariableTree> root, std::shared_ptr<AlarmLogic> alarmLogic, 
        const std::unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey>& _conditionMap);

private:
    void handle_value_change(const HashKey::EitherKey& property, std::pair<size_t, size_t> pairId);

    struct PairData {
        HashKey::EitherKey equipment;
        std::weak_ptr<VariableTree> root;
        std::weak_ptr<AlarmLogic> alarmLogic;
        std::shared_ptr<ValueListener> valueListener;
        std::mutex conditionMutex;
        std::unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey> conditions;
    };
    //root id, logic id
    std::unordered_map<size_t, std::unordered_map<size_t, PairData>> pairMap;
    std::weak_ptr<AlarmListener> listener;
};

#endif