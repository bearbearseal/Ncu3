#include "GlobalEnum.h"

using namespace std;

bool GlobalEnum::is_compare(int value)
{
    return value >= static_cast<int>(Compare::GREATER) && value <= static_cast<int>(Compare::NOTEQUAL);
}

bool GlobalEnum::is_schedule_action(int action)
{
    return action >= static_cast<int>(ScheduleAction::SET) && action <= static_cast<int>(ScheduleAction::WRITE);
}

bool GlobalEnum::is_schedule_subject(int subject)
{
    return (subject >= int(GlobalEnum::ScheduleSubject::WEEK_DAY) && subject <= int(GlobalEnum::ScheduleSubject::DDMMYYYY));
}
