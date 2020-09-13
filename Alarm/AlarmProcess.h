#ifndef _AlarmProcess_H_
#define _AlarmProcess_H_
#include <memory>
#include "../VariableTree/VariableTree.h"

class AlarmProcess {
public:
    AlarmProcess(std::shared_ptr<VariableTree> root);
    ~AlarmProcess();

};

#endif