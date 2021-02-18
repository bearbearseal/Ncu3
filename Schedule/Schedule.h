#ifndef _Schedule_H_
#define _Schedule_H_
#include <memory>
#include "TimeEventGun.h"
#include "../VariableTree/VariableTree.h"
#include "../../MyLib/Basic/Variable.h"
#include "../../MyLib/ITC/ITC.h"

//Has information of what day what schedule to follow
class Schedule {
public:
    Schedule();
    virtual ~Schedule();

    void add_write_event(const Value& value, uint8_t hour, uint8_t minute, uint8_t second);
    void add_set_event(const Value& value, uint8_t hour, uint8_t minute, uint8_t second);
    void add_unset_event(const Value& value, uint8_t hour, uint8_t minute, uint8_t second);

    void add_tree_node(std::weak_ptr<VariableTree> aNode);
    void remove_tree_node(const std::shared_ptr<VariableTree> aNode);

    void start();
    void stop();

private:
};

#endif