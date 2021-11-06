#include "OperationVariable.h"

using namespace std;

OperationVariable::OperationVariable(std::shared_ptr<OperationalLogic> _inLogic, std::shared_ptr<OperationalLogic> _outLogic) : inLogic(_inLogic), outLogic(_outLogic)
{
}

OperationVariable::~OperationVariable()
{
}

bool OperationVariable::write_value(const Value &newValue)
{
    if (outLogic != nullptr)
    {
        Value processed = outLogic->execute(newValue);
        return _write_value(processed);
    }
    return _write_value(newValue);
}

void OperationVariable::update_value_to_cache(const Value &newValue)
{
    if (inLogic != nullptr)
    {
        Value processed = inLogic->execute(newValue);
        Variable::update_value_to_cache(processed);
    }
    else
    {
        Variable::update_value_to_cache(newValue);
    }
}
