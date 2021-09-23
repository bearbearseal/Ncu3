#include "AlarmBuilder.h"

using namespace std;

unique_ptr<NodeAlarmManager> AlarmBuilder::create_node_alarm_manager(ConfigStorage &storage, std::shared_ptr<AlarmPostHandler> alarmPostHandler)
{
    unique_ptr<NodeAlarmManager> retVal = make_unique<NodeAlarmManager>(alarmPostHandler);
    auto logicList = storage.get_alarm_logic();
    printf("Got %zu logics\n", logicList.size());
    for (size_t i = 0; i < logicList.size(); ++i)
    {
        retVal->add_alarm_logic(logicList[i].id,
                                static_cast<AlarmDefinition::Comparison>(logicList[i].comparison),
                                logicList[i].referenceValue,
                                logicList[i].message,
                                static_cast<AlarmDefinition::AlarmState>(logicList[i].state),
                                logicList[i].code);
    }
    auto nodeAlarm = storage.get_node_alarm();
    printf("Got %zu node logic pairing.\n", nodeAlarm.size());
    for (size_t i = 0; i < nodeAlarm.size(); ++i)
    {
        retVal->set_node_logic(nodeAlarm[i].equipmentId, nodeAlarm[i].propertyId, nodeAlarm[i].alarmLogicId, nodeAlarm[i].priority);
    }
    auto normalMessage = storage.get_normal_message();
    printf("Got %zu normal message.\n", normalMessage.size());
    for (size_t i = 0; i < normalMessage.size(); ++i)
    {
        retVal->set_normal_message(normalMessage[i].equipmentId, normalMessage[i].propertyId, normalMessage[i].message);
    }
    return retVal;
}
