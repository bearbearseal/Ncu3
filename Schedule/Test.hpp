#include "ScheduleRule.h"
#include "TimeEventGun.h"

using namespace std;

namespace Test
{
    class Printfman : public TimeEventGun::Listener
    {
    public:
        Printfman(const std::string &_myName) { myName = _myName; }
        virtual ~Printfman() {}
        virtual void catch_time_event(uint8_t hour, uint8_t min, uint8_t sec, uint32_t token)
        {
            printf("%s caught event at %02u:%02u:%02u => %u\n", myName.c_str(), hour, min, sec, token);
        }
        virtual void catch_last_event(uint32_t token)
        {
            printf("%s caught last event => %u\n", myName.c_str(), token);
        }
        virtual void catch_time_forward(uint8_t hour, uint8_t min, uint8_t sec, uint32_t token)
        {
            printf("%s caught time forward.\n", myName.c_str());
        }
        virtual void catch_time_backward(uint8_t hour, uint8_t min, uint8_t sec, uint32_t token)
        {
            printf("%s caught time backward.\n", myName.c_str());
        }

    private:
        string myName;
    };

    void test_time_event_gun()
    {
        TimeEventGun timeEventGun;
        auto printfman1 = make_shared<Printfman>("1");
        auto printfman2 = make_shared<Printfman>("2");
        timeEventGun.add_event(22, 10, 22);
        timeEventGun.add_event(22, 10, 23);
        timeEventGun.add_listener(printfman1, 100);
        timeEventGun.add_listener(printfman2, 200);
        timeEventGun.start();
        while (true)
        {
            this_thread::sleep_for(1s);
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
} // namespace Test