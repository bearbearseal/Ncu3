#include "AlarmLogic.h"

using namespace std;

AlarmLogic::AlarmLogic(const std::string &comparison, const Value &value) : localValue(value)
{
    if (comparison == "==")
    {
        compare = Comparison::Equal;
    }
    else if (comparison == ">")
    {
        compare = Comparison::Greater;
    }
    else if (comparison == ">=")
    {
        compare = Comparison::GreaterEqual;
    }
    else if (comparison == "<=")
    {
        compare = Comparison::LesserEqual;
    }
    else if (comparison == "<")
    {
        compare = Comparison::Lesser;
    }
    else if (comparison == "!=")
    {
        compare = Comparison::NotEqual;
    }
    else
    {
        compare = Comparison::NotEqual;
    }
}

AlarmLogic::~AlarmLogic()
{
}

bool AlarmLogic::compare_true(const Value &value)
{
    switch (compare)
    {
    case Comparison::Greater:
        return value > localValue;
    case Comparison::GreaterEqual:
        return value >= localValue;
    case Comparison::Equal:
        return value == localValue;
    case Comparison::LesserEqual:
        return value <= localValue;
    case Comparison::Lesser:
        return value < localValue;
    case Comparison::NotEqual:
        return value != localValue;
    }
    return false;
}
