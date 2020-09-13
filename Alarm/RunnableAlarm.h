#ifndef _RunnableAlarm_H_
#define _RunnableAlarm_H_
#include "../VariableTree/VariableTree.h"

class RunnableAlarm {
public:
    RunnableAlarm();
    virtual ~RunnableAlarm();

private:
    uint32_t condition;
};

#endif