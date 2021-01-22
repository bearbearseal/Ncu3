#ifndef _Equipment_H_
#define _Equipment_H_
#include <memory>
#include <mutex>
#include "../VariableTree/VariableTree.h"
#include "../InOutOperation/OperationalLogic.h"

class Equipment : public VariableTree
{
private:
    class Property : public VariableTree
    {
    public:
        Property(const HashKey::EitherKey &_myId, std::shared_ptr<VariableTree::Parent> parentProxy);
        bool write_value(const Value &value, uint8_t priority = 0);
        bool set_value(const Value &value, uint8_t priority = 0);
    	Value get_value() const;
        //Do not call this while running, it is not protected from concurrent-access
        void set_in_out_operation(std::shared_ptr<OperationalLogic> _inOperation, std::shared_ptr<OperationalLogic> _outOperation);
    private:
        void catch_value_change(const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment);

        std::shared_ptr<OperationalLogic> inOperation;
        std::shared_ptr<OperationalLogic> outOperation;
        mutable std::mutex valueMutex;
        Value myValue;
    };

public:
    Equipment();
    ~Equipment();

	bool add_child(const HashKey::EitherKey& key, std::shared_ptr<VariableTree> newChild);
    std::shared_ptr<VariableTree> create_branch(const HashKey::EitherKey &key);
    std::shared_ptr<VariableTree> create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable);
    std::shared_ptr<VariableTree> force_create_branch(const HashKey::EitherKey &key);
    std::shared_ptr<VariableTree> force_create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable);

    bool set_in_out_operation(const HashKey::EitherKey& key, std::shared_ptr<OperationalLogic> inOperation, std::shared_ptr<OperationalLogic> outOperation);
};

#endif