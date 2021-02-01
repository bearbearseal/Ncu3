#include "Equipment.h"
#include "Property.h"

using namespace std;

Equipment::Equipment() : VariableTree(false)
{
}

Equipment::~Equipment()
{
}

std::shared_ptr<Equipment> Equipment::create_sub_equipment(const HashKey::EitherKey &key)
{
	shared_ptr<Equipment> retVal = make_shared<Equipment>();
    this->add_child(key, retVal);
    return retVal;
}


std::shared_ptr<VariableTree> Equipment::create_leaf(const HashKey::EitherKey &key, std::shared_ptr<Variable> _variable, std::shared_ptr<OperationalLogic> inOp, std::shared_ptr<OperationalLogic> outOp)
{
    shared_ptr<Property> property = make_shared<Property>(_variable, inOp, outOp);
    return VariableTree::create_leaf(key, property);
}
