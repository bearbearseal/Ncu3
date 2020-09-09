#include "PrioritizedValue.h"

using namespace std;

PrioritizedValue::PrioritizedValue()
{

}

PrioritizedValue::~PrioritizedValue()
{

}

bool PrioritizedValue::set_value(uint8_t priority, const Value& newValue)
{
    unique_lock<shared_mutex> lock(valueLock);
    auto i = valueMap.find(priority);
    if(i != valueMap.end())
    {
        if(newValue == i->second)
        {
            //This set value doesnt change anything
            return false;
        }
    }
    valueMap[priority] = newValue;
    if(priority == valueMap.rbegin()->first) //reverse begin is the highest priority
    {
        //This set value would change the get value's value.
        return true;
    }
    return false;
}

bool PrioritizedValue::unset_value(uint8_t priority)
{
    unique_lock<shared_mutex> lock(valueLock);
    if(!valueMap.size())
    {
        return false;
    }
    if(priority == valueMap.rbegin()->first)
    {
        //This unset would affect the read value
        valueMap.erase(priority);
        if(valueMap.size())
        {
            return true;
        }
        return false;
    }
    //This unset would not affect the read value
    valueMap.erase(priority);
    return false;
}

Value PrioritizedValue::get_value() const
{
    Value retValue;
    shared_lock<shared_mutex> lock(valueLock);
    auto i = valueMap.rbegin();
    if(i != valueMap.rend())
    {
        retValue = i->second;
    }
    return retValue;
}

bool PrioritizedValue::clear_value()
{
    unique_lock<shared_mutex> lock(valueLock);
    if(valueMap.size())
    {
        valueMap.clear();
        return true;
    }
    return false;
}
