#include "AlarmPostHandler.h"

using namespace std;

void AlarmPostHandler::pickup_alarm(AlarmDefinition::AlarmMessage &&alarm)
{
    printf("AlarmPostHandler picked up alarm, %s %d %s, %s.\n", alarm.activeValue.to_string().c_str(),
           static_cast<int>(alarm.comparison), alarm.referenceValue.to_string().c_str(), alarm.message.c_str());
}

void AlarmPostHandler::pickup_alarm(const AlarmDefinition::AlarmMessage &alarm)
{
    printf("AlarmPostHandler picked up alarm.\n");
}
