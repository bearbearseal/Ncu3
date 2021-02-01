#ifndef _Equipment_H_
#define _Equipment_H_
#include <memory>
#include <mutex>
#include "../VariableTree/VariableTree.h"
#include "../InOutOperation/OperationalLogic.h"

class Equipment : public VariableTree
{
public:
    Equipment();
    ~Equipment();

    std::shared_ptr<Equipment> create_sub_equipment(const HashKey::EitherKey &key);
    std::shared_ptr<VariableTree> create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable, std::shared_ptr<OperationalLogic> inOp, std::shared_ptr<OperationalLogic> outOp);
};

#endif