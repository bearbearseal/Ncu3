#ifndef _ScheduleRule_H_
#define _ScheduleRule_H_
#include <stdint.h>
#include <string>
#include <vector>

class ScheduleRule
{
public:
    enum class Subject
    {
        WEEK_DAY = 1, //Sunday is 0, Monday is 1
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
        YEAR_DAY,  //1st day is 1 instead of 0
        YEAR_WEEK, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        DDMM,
        DDMMYYYY,
        INVALID
    };
    static bool is_subject(int subject) { return (subject >= int(Subject::WEEK_DAY) && subject <= int(Subject::YEAR_WEEK)); }
    static bool subject_is_valid(Subject subject) { return subject != Subject::INVALID; }
    static Subject string_to_subject(const std::string &stringForm);

    enum class Comparison
    {
        GREATER = 1,
        GREATER_EQUAL,
        EQUAL,
        NOT_EQUAL,
        SMALLER_EQUAL,
        SMALLER,
        INVALID
    };
    static bool is_comparison(int compare) { return (compare >= int(Comparison::GREATER) && compare <= int(Comparison::SMALLER)); }
    static bool comparison_is_valid(Comparison comparison) { return comparison != Comparison::INVALID; }
    static Comparison string_to_comparison(const std::string &stringForm);

    struct Condition
    {
        Subject subject;
        Comparison compare;
        uint32_t value;
    };
    ScheduleRule();
    virtual ~ScheduleRule();

    //All the conditions in the list are AND relationship
    //RElationship between different list are OR condition
    void add_conditions(const std::vector<Condition> &_conditions);

    bool applicable_today() const;
    bool applicable(uint16_t year, uint8_t month, uint8_t day) const;
    bool applicable(const struct tm &theTm) const;

private:
    bool examine(const Condition &condition, const struct tm &theTm) const;
    bool condition_check(uint16_t subject, Comparison compare, uint16_t value) const;

    std::vector<std::vector<Condition>> conditionsList;
};

#endif