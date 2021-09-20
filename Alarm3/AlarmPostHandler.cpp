#include "AlarmPostHandler.h"

using namespace std;

void AlarmPostHandler::pickup_alarm(AlarmDefinition::AlarmMessage &&alarm)
{
    printf("AlarmPostHandler picked up alarm.\n");
}

void AlarmPostHandler::pickup_alarm(const AlarmDefinition::AlarmMessage &alarm)
{
    printf("AlarmPostHandler picked up alarm.\n");
}
