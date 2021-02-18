#ifndef _ScheduleRule_H_
#define _ScheduleRule_H_
#include <stdint.h>
#include <vector>

class ScheduleRule {
public:
    enum class Subject {
        WEEK_DAY,   //Sunday is 0, Monday is 1
        MONTH,
        MONTH_DAY,
        MONTH_WEEK, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        MONTH_SUNDAY,
        MONTH_MONDAY,
        MONTH_TUESDAY,
        MONTH_WEDNESDAY,
        MONTH_THURSDAY,
        MONTH_FRIDAY,
        MONTH_SATURDAY,
        YEAR,
        YEAR_DAY,   //1st day is 1 instead of 0
        YEAR_WEEK,  //1st week is 1 instead of 0, weeks with < 7 days is counted.
    };
    enum class Comparison {
        GREATER,
        GREATER_EQUAL,
        EQUAL,
        NOT_EQUAL,
        SMALLER_EQUAL,
        SMALLER
    };
    struct Condition {
        Subject subject;
        Comparison compare;
        uint16_t value;
    };
    ScheduleRule();
    virtual ~ScheduleRule();

    void add_conditions(const std::vector<Condition>& _conditions);

    bool applicable_today() const;
    bool applicable(uint16_t year, uint8_t month, uint8_t day) const;

private:
    bool applicable(const struct tm& theTm) const;
    bool examine(const Condition& condition, const struct tm& theTm) const;
    bool condition_check(uint16_t subject, Comparison compare, uint16_t value) const;

    std::vector<std::vector<Condition>> conditionsList;
};

#endif