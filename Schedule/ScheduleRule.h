/*
Schedule Rule stores logic to determine if a schedule is applicable at specific date.
*/
#ifndef _ScheduleRule_H_
#define _ScheduleRule_H_
#include <stdint.h>
#include <string>
#include <vector>
#include "../Global/GlobalEnum.h"

class ScheduleRule
{
public:
    struct Condition
    {
        GlobalEnum::ScheduleSubject subject;
        GlobalEnum::Compare compare;
        uint32_t value;
    };
    ScheduleRule();
    ScheduleRule(std::vector<std::vector<Condition>>&& conditionList);
    virtual ~ScheduleRule();

    //All the conditions in the list are AND relationship
    //RElationship between different list are OR condition
    void add_conditions(const std::vector<Condition> &_conditions);

    bool applicable_today() const;
    bool applicable(uint16_t year, uint8_t month, uint8_t day) const;
    bool applicable(const struct tm &theTm) const;

private:
    bool examine(const Condition &condition, const struct tm &theTm) const;
    bool condition_check(uint16_t subject, GlobalEnum::Compare compare, uint16_t value) const;

    std::vector<std::vector<Condition>> conditionsList;
};

#endif