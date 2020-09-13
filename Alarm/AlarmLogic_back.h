#ifndef _AlarmLogic_H_
#define _AlarmLogic_H_
#include <memory>
#include <unordered_map>
#include <map>
#include <mutex>
#include "../VariableTree/VariableTree.h"
#include "../Basic/HashKey.h"
#include "AlarmListener.h"

//Higher priority got higher number
class AlarmLogic {
public:
    enum class Comparison {
        GREATER,
        GREATER_EQUAL,
        EQUAL,
        SMALLER_EQUAL,
        SMALLER
    };
    AlarmLogic();
    virtual ~AlarmLogic();

    void add_subject(std::shared_ptr<VariableTree> root);
    void set_listener(std::shared_ptr<AlarmListener> listener);
    void set_condition(const HashKey::EitherKey& leftProperty, uint32_t condition);
    void add_logic(const HashKey::EitherKey& leftProperty, const HashKey::EitherKey& rightProperty2, Comparison compare, uint32_t condition, uint8_t priority);
    void add_logic(const HashKey::EitherKey& leftProperty1, const Value& constant, Comparison compare, uint32_t condition, uint8_t priority);
    void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

private:
    class Shadow : public VariableTree::ValueChangeListener {
    public:
        Shadow(AlarmLogic& _master, size_t _subjectId) : master(_master), subjectId(_subjectId) {}
        virtual ~Shadow() {}
        void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { 
            master.catch_value_change_event(branch, newValue, theMoment);
        }

    private:
        AlarmLogic& master;
        size_t subjectId;
    };

    struct RootData {
        std::weak_ptr<VariableTree> root;
        std::shared_ptr<Shadow> logicShadow;
        std::unordered_map<HashKey::EitherKey, uint32_t, HashKey::EitherKey> conditions;
    };

    struct LogicData {
        LogicData(const HashKey::EitherKey& _rightProperty, const Value& constant, Comparison compare, uint16_t condition);
        const HashKey::EitherKey rightProperty;
        const Value constant;
        Comparison compare;
        uint32_t condition;
    };
    struct PropertyData {
        uint32_t activeCondition = 0;
        std::map<uint8_t, LogicData> logicData;
    };
    struct SubjectData {
        std::shared_ptr<Shadow> myShadow;
        std::weak_ptr<VariableTree> subject;
        
    };

    bool examine_alarm(const Value& left, const Value& right, Comparison compare);

    std::mutex mapMutex;
    std::unordered_map<HashKey::EitherKey, PropertyData, HashKey::EitherKey> property2Priority2Data;
    std::unordered_map<HashKey::EitherKey, std::unordered_set<HashKey::EitherKey, HashKey::EitherKey>, HashKey::EitherKey> rightProperty2Affected;
    std::weak_ptr<VariableTree> subject;
    std::weak_ptr<AlarmListener> listener;
    std::shared_ptr<Shadow> myShadow;
};

#endif