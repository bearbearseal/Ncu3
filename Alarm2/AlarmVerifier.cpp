#include "AlarmVerifier.h"

using namespace std;
using namespace AlarmDefinition;

AlarmVerifier::AlarmVerifier(AlarmDefinition::Comparison _compare, const Value &theValue) : compareValue(theValue)
{
    compare = _compare;
}

AlarmVerifier::~AlarmVerifier()
{
}

bool AlarmVerifier::verify(const Value &verifee)
{
    switch (compare)
    {
    case Comparison::GREATER:
        return verifee > compareValue;
    case Comparison::GREATER_EQUAL:
        return verifee >= compareValue;
    case Comparison::EQUAL:
        return verifee == compareValue;
    case Comparison::SMALLER_EQUAL:
        return verifee <= compareValue;
    case Comparison::SMALLER:
        return verifee < compareValue;
    case Comparison::NOT_EQUAL:
        return verifee != compareValue;
    }
    return false;
}