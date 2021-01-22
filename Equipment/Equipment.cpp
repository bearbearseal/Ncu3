#include "Equipment.h"

using namespace std;

Equipment::Property::Property(const HashKey::EitherKey &_myId, std::shared_ptr<VariableTree::Parent> parentProxy) : VariableTree(_myId, parentProxy, true)
{
}

bool Equipment::Property::write_value(const Value &value, uint8_t priority)
{
    if(outOperation == nullptr) {
        return VariableTree::write_value(value, priority);
    }
    return VariableTree::write_value(outOperation->execute(value), priority);
}

bool Equipment::Property::set_value(const Value &value, uint8_t priority)
{
    if(outOperation == nullptr) {
        return VariableTree::set_value(value, priority);
    }
    return VariableTree::set_value(outOperation->execute(value), priority);
}

Value Equipment::Property::get_value() const {
    lock_guard<mutex> lock(valueMutex);
    return myValue;
}

void Equipment::Property::set_in_out_operation(std::shared_ptr<OperationalLogic> _inOperation, std::shared_ptr<OperationalLogic> _outOperation) {
    inOperation = _inOperation;
    outOperation = _outOperation;
}


void Equipment::Property::catch_value_change(const Value &newValue, std::chrono::time_point<std::chrono::system_clock> theMoment)
{
    if(inOperation == nullptr) {
        lock_guard<mutex> lock(valueMutex);
        myValue = newValue;
    }
    else {
        lock_guard<mutex> lock(valueMutex);
        myValue = inOperation->execute(newValue);
    }
    VariableTree::catch_value_change(myValue, theMoment);
}

Equipment::Equipment() : VariableTree(false)
{
    
}

Equipment::~Equipment()
{
}

//Prevent non-property child from being added
bool Equipment::add_child(const HashKey::EitherKey& key, std::shared_ptr<VariableTree> newChild) {
    return false;
}

std::shared_ptr<VariableTree> Equipment::create_branch(const HashKey::EitherKey &key)
{
	shared_ptr<VariableTree> retVal = make_shared<Equipment>();
    VariableTree::add_child(key, retVal);
    return retVal;
}

std::shared_ptr<VariableTree> Equipment::create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable)
{
    shared_ptr<VariableTree> retVal = make_shared<Property>(key, toChildren);
    VariableTree::add_child(key, retVal);
    return retVal;
}

std::shared_ptr<VariableTree> Equipment::force_create_branch(const HashKey::EitherKey &key)
{
    return nullptr;
}

std::shared_ptr<VariableTree> Equipment::force_create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable)
{
    return nullptr;
}

bool Equipment::set_in_out_operation(const HashKey::EitherKey &key, std::shared_ptr<OperationalLogic> inOperation, std::shared_ptr<OperationalLogic> outOperation)
{
    shared_ptr<Property> child = dynamic_pointer_cast<Property>(this->get_child(key));
    if(child != nullptr) {
        child->set_in_out_operation(inOperation, outOperation);
        return true;
    }
    return false;
}
