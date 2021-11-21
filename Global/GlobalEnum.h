#ifndef _GlobalEnum_H_
#define _GlobalEnum_H_
#include <stdint.h>

namespace GlobalEnum {
    enum class Compare
    {
        GREATER         = 1,
        GREATEREQUAL    = 2,
        EQUAL           = 3,
        SMALLEREQUAL    = 4,
        SMALLER         = 5,
        NOTEQUAL        = 6,
        INVALID
    };
    bool is_compare(int value)
    {
        return value>=static_cast<int>(Compare::GREATER) && value<=static_cast<int>(Compare::NOTEQUAL);
    }
    enum class ScheduleAction {
        SET     = 1,
        UNSET   = 2,
        WRITE   = 3,
        INVALID
    };
    bool is_schedule_action(int action)
    {
        return action>=static_cast<int>(ScheduleAction::SET) && action<=static_cast<int>(ScheduleAction::WRITE);
    }
    enum class ScheduleSubject
    {
        WEEK_DAY        = 1, //Sunday is 0, Monday is 1
        MONTH           = 2,
        MONTH_DAY       = 3,
        MONTH_WEEK      = 4, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        MONTH_SUNDAY    = 5,
        MONTH_MONDAY    = 6,
        MONTH_TUESDAY   = 7,
        MONTH_WEDNESDAY = 8,
        MONTH_THURSDAY  = 9,
        MONTH_FRIDAY    = 10,
        MONTH_SATURDAY  = 11,
        YEAR            = 12,
        YEAR_DAY        = 13, //1st day is 1 instead of 0
        YEAR_WEEK       = 14, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        DDMM            = 15,
        DDMMYYYY        = 16,
        INVALID
    };
    bool is_schedule_subject(int subject)
    {
        return (subject >= int(GlobalEnum::ScheduleSubject::WEEK_DAY) && subject <= int(GlobalEnum::ScheduleSubject::DDMMYYYY));
    }
}

#endif