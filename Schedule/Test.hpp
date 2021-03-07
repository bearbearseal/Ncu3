#include "ScheduleRule.h"
#include "ScheduleFunction.h"
#include "TimeTable.h"
#include "Schedule.h"
#include "Timer.h"

using namespace std;

class ScheduleListener : public Schedule::Listener
{
public:
    ScheduleListener() {}
    virtual ~ScheduleListener() {}
    virtual void catch_set_event(const Value &setValue)
    {
        printf("Caught set event, value: %s.\n", setValue.to_string().c_str());
    }
    virtual void catch_unset_event()
    {
        printf("Caught unset event.\n");
    }
    virtual void catch_write_event(const Value &setValue)
    {
        printf("Caught write event, value: %s.\n", setValue.to_string().c_str());
    }
};

class TimerListener : public Timer::Listener
{
public:
    void catch_time_event(time_t eventTime, uint32_t token) 
    {
        auto hms = ScheduleFunction::day_time_to_hms(eventTime);
        printf("Timer listenr caught event at %02u:%02u:%02u-%u\n", hms.hour, hms.minute, hms.second, token);
    }

};

namespace Test
{
    void test_timer()
    {
        shared_ptr<TimerListener> listener = make_shared<TimerListener>();
        shared_ptr<Timer> timer = make_shared<Timer>();
        timer->add_time_event(ScheduleFunction::hms_to_local_time_t(16, 47, 10), listener, 10);
        timer->add_time_event(ScheduleFunction::hms_to_local_time_t(21, 34, 10), listener, 20);
        timer->add_time_event(ScheduleFunction::hms_to_local_time_t(21, 34, 30), listener, 30);
        timer->add_time_event(ScheduleFunction::hms_to_local_time_t(21, 34, 40), listener, 40);
        timer->add_time_event(ScheduleFunction::hms_to_local_time_t(21, 34, 50), listener, 50);
        printf("Event added.\n");
        time_t baseTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        while(1)
        {
            baseTime += 7;
            this_thread::sleep_for(5s);
            timer->add_time_event(baseTime, listener, 5);
        }
    }

    void test_schedule_rule()
    {
        ScheduleRule sr1;
        vector<ScheduleRule::Condition> conditionSunday;
        conditionSunday.push_back({ScheduleRule::Subject::WEEK_DAY, ScheduleRule::Comparison::EQUAL, 0});
        sr1.add_conditions(conditionSunday);
        vector<ScheduleRule::Condition> conditionSaturday;
        conditionSaturday.push_back({ScheduleRule::Subject::WEEK_DAY, ScheduleRule::Comparison::EQUAL, 6});
        sr1.add_conditions(conditionSaturday);
        vector<ScheduleRule::Condition> conditionChristmas;
        conditionChristmas.push_back({ScheduleRule::Subject::MONTH, ScheduleRule::Comparison::EQUAL, 12});
        conditionChristmas.push_back({ScheduleRule::Subject::MONTH_DAY, ScheduleRule::Comparison::EQUAL, 25});
        sr1.add_conditions(conditionChristmas);
        vector<ScheduleRule::Condition> conditionLaborDay;
        conditionLaborDay.push_back({ScheduleRule::Subject::MONTH, ScheduleRule::Comparison::EQUAL, 5});
        conditionLaborDay.push_back({ScheduleRule::Subject::MONTH_DAY, ScheduleRule::Comparison::EQUAL, 1});
        sr1.add_conditions(conditionLaborDay);

        ScheduleRule sr2;
        vector<ScheduleRule::Condition> conditionSpring;
        conditionSpring.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::GREATER_EQUAL, 79});
        conditionSpring.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::SMALLER_EQUAL, 171});
        sr2.add_conditions(conditionSpring);
        vector<ScheduleRule::Condition> conditionFall;
        conditionFall.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::GREATER_EQUAL, 264});
        conditionFall.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::SMALLER_EQUAL, 256});
        sr2.add_conditions(conditionFall);

        ScheduleRule sr3;
        vector<ScheduleRule::Condition> condition1stSat;
        condition1stSat.push_back({ScheduleRule::Subject::MONTH_SATURDAY, ScheduleRule::Comparison::EQUAL, 1});
        sr3.add_conditions(condition1stSat);
        vector<ScheduleRule::Condition> condition3rdSat;
        condition3rdSat.push_back({ScheduleRule::Subject::MONTH_SATURDAY, ScheduleRule::Comparison::EQUAL, 3});
        sr3.add_conditions(condition3rdSat);

        printf("%s\n", sr1.applicable_today() ? "yes" : "no");
        printf("%s\n", sr1.applicable(2021, 03, 31) ? "yes" : "no");
        printf("%s\n", sr1.applicable(2020, 12, 25) ? "yes" : "no");
        printf("%s\n", sr1.applicable(2023, 5, 1) ? "yes" : "no");
        printf("%s\n", sr1.applicable(2020, 9, 13) ? "yes" : "no");
        printf("%s\n", sr1.applicable(2021, 1, 2) ? "yes" : "no");
        printf("\n");

        printf("%s\n", sr2.applicable_today() ? "yes" : "no");
        printf("%s\n", sr2.applicable(2021, 03, 31) ? "yes" : "no");
        printf("%s\n", sr2.applicable(2020, 12, 25) ? "yes" : "no");
        printf("%s\n", sr2.applicable(2023, 5, 1) ? "yes" : "no");
        printf("%s\n", sr2.applicable(2020, 9, 13) ? "yes" : "no");
        printf("%s\n", sr2.applicable(2021, 1, 2) ? "yes" : "no");
        printf("\n");

        printf("%s\n", sr3.applicable_today() ? "yes" : "no");
        printf("%s\n", sr3.applicable(2021, 8, 31) ? "yes" : "no");
        printf("%s\n", sr3.applicable(2014, 9, 6) ? "yes" : "no");  //1st saturday
        printf("%s\n", sr3.applicable(2021, 2, 20) ? "yes" : "no"); //3rd sat
        printf("\n");
    }

    void test_schedule()
    {
        ScheduleRule sr1;
        vector<ScheduleRule::Condition> conditionSunday;
        conditionSunday.push_back({ScheduleRule::Subject::WEEK_DAY, ScheduleRule::Comparison::EQUAL, 0});
        sr1.add_conditions(conditionSunday);
        vector<ScheduleRule::Condition> conditionSaturday;
        conditionSaturday.push_back({ScheduleRule::Subject::WEEK_DAY, ScheduleRule::Comparison::EQUAL, 6});
        sr1.add_conditions(conditionSaturday);
        vector<ScheduleRule::Condition> conditionChristmas;
        conditionChristmas.push_back({ScheduleRule::Subject::MONTH, ScheduleRule::Comparison::EQUAL, 12});
        conditionChristmas.push_back({ScheduleRule::Subject::MONTH_DAY, ScheduleRule::Comparison::EQUAL, 25});
        sr1.add_conditions(conditionChristmas);
        vector<ScheduleRule::Condition> conditionLaborDay;
        conditionLaborDay.push_back({ScheduleRule::Subject::MONTH, ScheduleRule::Comparison::EQUAL, 5});
        conditionLaborDay.push_back({ScheduleRule::Subject::MONTH_DAY, ScheduleRule::Comparison::EQUAL, 1});
        sr1.add_conditions(conditionLaborDay);

        ScheduleRule sr2;
        vector<ScheduleRule::Condition> conditionSpring;
        conditionSpring.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::GREATER_EQUAL, 79});
        conditionSpring.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::SMALLER_EQUAL, 171});
        sr2.add_conditions(conditionSpring);
        vector<ScheduleRule::Condition> conditionFall;
        conditionFall.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::GREATER_EQUAL, 264});
        conditionFall.push_back({ScheduleRule::Subject::YEAR_DAY, ScheduleRule::Comparison::SMALLER_EQUAL, 256});
        sr2.add_conditions(conditionFall);

        ScheduleRule sr3;
        vector<ScheduleRule::Condition> condition1stSat;
        condition1stSat.push_back({ScheduleRule::Subject::MONTH_SATURDAY, ScheduleRule::Comparison::EQUAL, 1});
        sr3.add_conditions(condition1stSat);
        vector<ScheduleRule::Condition> condition3rdSat;
        condition3rdSat.push_back({ScheduleRule::Subject::MONTH_SATURDAY, ScheduleRule::Comparison::EQUAL, 3});
        sr3.add_conditions(condition3rdSat);

        shared_ptr<TimeTable> timeTable1 = make_shared<TimeTable>();
        timeTable1->add_interval(50, {11, 11, 11}, {11, 12, 12});
        timeTable1->add_write_event(20, {11, 12, 30});

        shared_ptr<TimeTable> timeTable2 = make_shared<TimeTable>();
        timeTable2->add_interval(0, {20, 00, 00}, {20, 00, 59});
        timeTable2->add_interval(1, {20, 01, 00}, {20, 01, 59});
        timeTable2->add_interval(0, {20, 02, 00}, {20, 02, 59});
        timeTable2->add_interval(1, {20, 03, 00}, {20, 03, 59});
        timeTable2->add_interval(0, {20, 04, 00}, {20, 04, 59});
        timeTable2->add_interval(1, {20, 05, 00}, {20, 05, 59});
        timeTable2->add_interval(0, {20, 06, 00}, {20, 06, 59});
        timeTable2->add_interval(1, {20, 07, 00}, {20, 07, 59});

        shared_ptr<TimeTable> timeTable3 = make_shared<TimeTable>();
        timeTable3->add_interval(2, {20, 00, 30}, {20, 00, 50});
        timeTable3->add_interval(3, {20, 01, 30}, {20, 01, 50});
        timeTable3->add_interval(2, {20, 02, 30}, {20, 02, 50});
        timeTable3->add_interval(3, {20, 03, 30}, {20, 03, 50});

        shared_ptr<Timer> timer = make_shared<Timer>();
        Schedule schedule1(timer);
        schedule1.set_default_time_table(timeTable1);
        schedule1.add_time_table(timeTable2, sr1, 1);
        schedule1.add_time_table(timeTable3, sr2, 1);

        shared_ptr<ScheduleListener> listener1 = make_shared<ScheduleListener>();
        //schedule1.add_listener(listener1);
        printf("Schedule1 starting.\n");
        schedule1.start();
        while (1)
        {
            this_thread::sleep_for(1s);
        }
    }
} // namespace Test