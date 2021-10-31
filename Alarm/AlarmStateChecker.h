#ifndef AlarmStateChecker_H_
#define AlarmStateChecker_H_
#include "../../MyLib/Basic/Value.h"
#include "AlarmDefinition.h"

class AlarmStateChecker
{
public:
    AlarmStateChecker(AlarmDefinition::Comparison compare, const Value &theValue);
    virtual ~AlarmStateChecker();

    bool verify(const Value &verifee) const;
    inline AlarmDefinition::Comparison get_compare() const { return compare; }
    inline const Value &get_value() const { return compareValue; }

private:
    AlarmDefinition::Comparison compare;
    const Value compareValue;
};

#endif