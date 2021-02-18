#ifndef _ScheduleManager_H_
#define _ScheduleManager_H_
#include <memory>
#include "Schedule.h"
#include "../VariableTree/VariableTree.h"

class ScheduleManager {
public:
    ScheduleManager();
    virtual ~ScheduleManager();

    uint32_t add_schedule(std::unique_ptr<Schedule> newSchedule);

    bool pair_node_with_schedule(std::weak_ptr<VariableTree> node, uint32_t scheduleId, const std::string& rules, uint8_t priority);
};

#endif