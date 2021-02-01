#include <thread>
#include "Property.h"

using namespace std;

Property::Property(std::shared_ptr<Variable> _theVariable, std::shared_ptr<OperationalLogic> _inOp, std::shared_ptr<OperationalLogic> _outOp)
{
    me = make_shared<Shadow>(*this);
    theVariable = _theVariable;
    theVariable->add_listener(me);
    inOperation = _inOp;
    outOperation = _outOp;
}

Property::~Property()
{
    weak_ptr<Shadow> weak = me;
    me.reset();
    while(weak.lock() != nullptr)
    {
        this_thread::yield();
    }
}

bool Property::write_value(const Value &newValue)
{
    if(outOperation != nullptr) {
        return theVariable->write_value(outOperation->execute(newValue));
    }
    return theVariable->write_value(newValue);
}

void Property::catch_value_change(const Value &newValue)
{
    if(inOperation != nullptr) {
        update_value_to_cache(inOperation->execute(newValue));
    }
    else {
        update_value_to_cache(newValue);
    }
}
