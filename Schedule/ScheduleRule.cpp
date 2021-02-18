#include "ScheduleRule.h"
#include <chrono>
#include <stdio.h>
#include <string.h>

using namespace std;

ScheduleRule::ScheduleRule()
{
}

ScheduleRule::~ScheduleRule()
{
}

void ScheduleRule::add_conditions(const std::vector<Condition> &_conditions)
{
    if(_conditions.size()) {
        conditionsList.push_back(_conditions);
    }
}

bool ScheduleRule::applicable_today() const
{
    time_t totalSec = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm theTm;
    localtime_r(&totalSec, &theTm);
    return applicable(theTm);
}

bool ScheduleRule::applicable(uint16_t year, uint8_t month, uint8_t day) const
{
    tm theTm;
    memset(&theTm, 0x00, sizeof(theTm));
    theTm.tm_year = year - 1900;
    theTm.tm_mon = month - 1;
    theTm.tm_mday = day;
    time_t totalSec = 0;
    totalSec = mktime(&theTm);
    localtime_r(&totalSec, &theTm);
    return applicable(theTm);
}

bool ScheduleRule::applicable(const struct tm &theTm) const
{
    bool retVal = false;
    for (size_t i = 0; i < conditionsList.size(); ++i)
    {
        bool subResult = true;
        for (size_t j = 0; j < conditionsList[i].size() && subResult; ++j)
        {
            subResult &= examine(conditionsList[i][j], theTm);
        }
        retVal |= subResult;
    }
    //printf("Returning %s\n", retVal ? "true" : "false");
    return retVal;
}

bool ScheduleRule::examine(const Condition &condition, const struct tm &theTm) const
{
    uint16_t subject;
    switch (condition.subject)
    {
    case Subject::WEEK_DAY:
        subject = theTm.tm_wday;
        break;
    case Subject::MONTH:
        //printf("Month %u\n", theTm.tm_mon + 1);
        subject = theTm.tm_mon + 1;
        break;
    case Subject::MONTH_DAY:
        //printf("Month day %u\n", theTm.tm_mday);
        subject = theTm.tm_mday;
        break;
    case Subject::MONTH_WEEK:
        subject = theTm.tm_mday + (6 - theTm.tm_wday);
        subject /= 7;
        if (subject % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_SUNDAY:
        if (!theTm.tm_wday)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_MONDAY:
        if (theTm.tm_wday != 1)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_TUESDAY:
        if (theTm.tm_wday != 2)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_WEDNESDAY:
        if (theTm.tm_wday != 3)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_THURSDAY:
        if (theTm.tm_wday != 4)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_FRIDAY:
        if (theTm.tm_wday != 5)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::MONTH_SATURDAY:
        if (theTm.tm_wday != 6)
        {
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case Subject::YEAR:
        subject = theTm.tm_year;
        subject += 1900;
        break;
    case Subject::YEAR_DAY:
        subject = theTm.tm_yday;
        break;
    case Subject::YEAR_WEEK:
        subject = (theTm.tm_yday + 6 - theTm.tm_wday);
        subject /= 7;
        if (subject % 7)
        {
            ++subject;
        }
        break;
    }
    return condition_check(subject, condition.compare, condition.value);
}

bool ScheduleRule::condition_check(uint16_t subject, Comparison compare, uint16_t value) const
{
    switch (compare)
    {
    case Comparison::GREATER:
        return subject > value;
    case Comparison::GREATER_EQUAL:
        return subject >= value;
    case Comparison::EQUAL:
        //printf("Checking if %u equals %u\n", subject, value);
        return subject == value;
    case Comparison::NOT_EQUAL:
        return subject != value;
    case Comparison::SMALLER_EQUAL:
        return subject <= value;
    case Comparison::SMALLER:
        return subject < value;
    }
    return false;
}
