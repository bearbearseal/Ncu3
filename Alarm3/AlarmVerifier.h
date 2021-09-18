#ifndef AlarmVerifier_H_
#define AlarmVerifier_H_
#include "../../MyLib/Basic/Value.h"
#include "AlarmDefinition.h"

class AlarmVerifier
{
public:
    AlarmVerifier(AlarmDefinition::Comparison compare, const Value &theValue);
    virtual ~AlarmVerifier();

    bool verify(const Value &verifee);
    inline AlarmDefinition::Comparison get_compare() { return compare; }
    inline const Value &get_value() { return compareValue; }

private:
    AlarmDefinition::Comparison compare;
    const Value compareValue;
};

#endif