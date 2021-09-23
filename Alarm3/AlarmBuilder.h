#ifndef _AlarmBuilder_H_
#define _AlarmBuilder_H_
#include <memory>
#include "NodeAlarmManager.h"
#include "../Storage/ConfigStorage.h"
#include "AlarmPostHandler.h"

namespace AlarmBuilder
{
    std::unique_ptr<NodeAlarmManager> create_node_alarm_manager(ConfigStorage& storage, std::shared_ptr<AlarmPostHandler> alarmPostHandler);
}

#endif