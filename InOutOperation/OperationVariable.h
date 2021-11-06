#ifndef _OperationVariable_H_
#define _OperationVariable_H_
#include <memory>
#include "../../MyLib/Basic/Variable.h"
#include "OperationalLogic.h"

class OperationVariable : public Variable
{
public:
    OperationVariable(std::shared_ptr<OperationalLogic> _inLogic, std::shared_ptr<OperationalLogic> _outLogic);
    virtual ~OperationVariable();
	bool write_value(const Value& newValue);

protected:
    bool virtual _write_value(const Value& newValue) = 0;
	virtual void update_value_to_cache(const Value& newValue);

private:
    std::shared_ptr<OperationalLogic> inLogic;
    std::shared_ptr<OperationalLogic> outLogic;
};

#endif