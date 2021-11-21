#include "ScheduleRule.h"
#include <chrono>
#include <stdio.h>
#include <string.h>
#include <algorithm>

using namespace std;

ScheduleRule::ScheduleRule()
{
}

ScheduleRule::ScheduleRule(vector<vector<Condition>>&& _conditionList)
{
    conditionsList = _conditionList;
}

ScheduleRule::~ScheduleRule()
{
}

void ScheduleRule::add_conditions(const std::vector<Condition> &_conditions)
{
    if (_conditions.size())
    {
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
    case GlobalEnum::ScheduleSubject::WEEK_DAY:
        subject = theTm.tm_wday;
        break;
    case GlobalEnum::ScheduleSubject::MONTH:
        //printf("Month %u\n", theTm.tm_mon + 1);
        subject = theTm.tm_mon + 1;
        break;
    case GlobalEnum::ScheduleSubject::MONTH_DAY:
        //printf("Month day %u\n", theTm.tm_mday);
        subject = theTm.tm_mday;
        break;
    case GlobalEnum::ScheduleSubject::MONTH_WEEK:
        subject = theTm.tm_mday + (6 - theTm.tm_wday);
        subject /= 7;
        if (subject % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_SUNDAY:
        if (theTm.tm_wday)
        {
            //printf("Not Sunday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_MONDAY:
        if (theTm.tm_wday != 1)
        {
            //printf("Not Monday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_TUESDAY:
        if (theTm.tm_wday != 2)
        {
            //printf("Not Tuesday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_WEDNESDAY:
        if (theTm.tm_wday != 3)
        {
            //printf("Not Wednesday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_THURSDAY:
        if (theTm.tm_wday != 4)
        {
            //printf("Not Thursday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_FRIDAY:
        if (theTm.tm_wday != 5)
        {
            //printf("Not Friday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::MONTH_SATURDAY:
        if (theTm.tm_wday != 6)
        {
            //printf("Not Saturday, not applicable.\n");
            return false;
        }
        subject = theTm.tm_mday / 7;
        if (theTm.tm_mday % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::YEAR:
        subject = theTm.tm_year;
        subject += 1900;
        break;
    case GlobalEnum::ScheduleSubject::YEAR_DAY:
        subject = theTm.tm_yday;
        break;
    case GlobalEnum::ScheduleSubject::YEAR_WEEK:
        subject = (theTm.tm_yday + 6 - theTm.tm_wday);
        subject /= 7;
        if (subject % 7)
        {
            ++subject;
        }
        break;
    case GlobalEnum::ScheduleSubject::DDMM:
        subject = theTm.tm_mday;
        subject *= 100;
        subject += (theTm.tm_mon + 1);
        break;
    case GlobalEnum::ScheduleSubject::DDMMYYYY:
        subject = theTm.tm_mday;
        subject *= 100;
        subject += (theTm.tm_mon + 1);
        subject *= 10000;
        subject += (theTm.tm_year + 1900);
        break;

    default:
        //printf("Default, not applicable.\n");
        return false;
    }
    bool checkResult = condition_check(subject, condition.compare, condition.value);
    //printf("Result is %s\n", checkResult ? "true" : "false");
    return checkResult;
}

bool ScheduleRule::condition_check(uint16_t subject, GlobalEnum::Compare compare, uint16_t value) const
{
    //printf("Condition check, subject %u compare %d value %u\n", subject, int(compare), value);
    switch (compare)
    {
    case GlobalEnum::Compare::GREATER:
        return subject > value;
    case GlobalEnum::Compare::GREATEREQUAL:
        return subject >= value;
    case GlobalEnum::Compare::EQUAL:
        //printf("Checking if %u equals %u\n", subject, value);
        return subject == value;
    case GlobalEnum::Compare::NOTEQUAL:
        return subject != value;
    case GlobalEnum::Compare::SMALLEREQUAL:
        return subject <= value;
    case GlobalEnum::Compare::SMALLER:
        return subject < value;
    default:
        return false;
    }
    return false;
}
