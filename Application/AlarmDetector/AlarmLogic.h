//Takes in input, verify if it's a true
#ifndef AlarmLogic_H_
#define AlarmLogic_H_
#include <string>
#include "../../../MyLib/Basic/Value.h"

class AlarmLogic
{
public:
    AlarmLogic(const std::string &comparison, const Value &value);
    virtual ~AlarmLogic();
    bool compare_true(const Value &value);

private:
    enum class Comparison
    {
        Greater,
        GreaterEqual,
        Equal,
        LesserEqual,
        Lesser,
        NotEqual
    };
    const Value localValue;
    Comparison compare;
};

#endif