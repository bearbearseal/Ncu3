#ifndef _AlarmHandler_H_
#define _AlarmHandler_H_
#include <memory>
#include "AlarmLogic.h"
#include "../VariableTree/VariableTree.h"

class AlarmHandler {
public:
    AlarmHandler(std::unique_ptr<AlarmLogic> alarmLogic);
    virtual ~AlarmHandler();

    void add_root(std::shared_ptr<VariableTree> root);

    void catch_tree_value_change(size_t rootId, const HashKey::EitherKey& property);
    class ValueListener : public VariableTree::ValueChangeListener {
    public:
        ValueListener(AlarmHandler& _master, size_t _rootId) : master(_master), rootId(_rootId) {}
        virtual ~ValueListener() {}
		virtual void catch_value_change_event(const std::vector<HashKey::EitherKey>& branch, const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

    private:
        AlarmHandler& master;
        size_t rootId;
    };
};

#endif