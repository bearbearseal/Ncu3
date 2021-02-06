#ifndef _Property_H_
#define _Property_H_
#include <memory>

#include "../../MyLib/Basic/Variable.h"
#include "../InOutOperation/OperationalLogic.h"

class Property : public Variable
{
private:
    class Shadow : public Variable::Listener {
    public:
        Shadow(Property& _me) : me(_me) {}
        virtual ~Shadow() {}
		void catch_value_change(const Value& newValue, std::chrono::time_point<std::chrono::system_clock> theMoment) { me.catch_value_change(newValue); }
        
    private:
        Property& me;
    };

public:
    Property(std::shared_ptr<Variable> _theVariable, std::shared_ptr<OperationalLogic> _inOp, std::shared_ptr<OperationalLogic> _outOp);
    virtual ~Property();
	bool write_value(const Value& newValue);
    void catch_value_change(const Value& newValue);

private:
    std::shared_ptr<Shadow> me;

    std::shared_ptr<OperationalLogic> inOperation;
    std::shared_ptr<OperationalLogic> outOperation;
    std::shared_ptr<Variable> theVariable;
};

#endif