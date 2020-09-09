#ifndef _PrioritizedValue_H_
#define _PrioritizedValue_H_
#include "Value.h"
#include <map>
#include <shared_mutex>

//Higher number => higher priority
class PrioritizedValue {
public:
    PrioritizedValue();
    virtual ~PrioritizedValue();

    //Returns true if this set triggers change of read value
    bool set_value(uint8_t priority, const Value& newValue);
    //Returns true if this unset triggers change of read value
    bool unset_value(uint8_t priority);
    //Simular to unset at all priorities value 
    bool clear_value();
    Value get_value() const;
    //Does not persist, same as clear then set with priority 0
    void trigger_value(const Value& newValue) { clear_value(); set_value(0, newValue); }

private:
    mutable std::shared_mutex valueLock;
    std::map<uint8_t, Value> valueMap;
};
#endif