#ifndef _PointLogger_H_
#define _PointLogger_H_
#include <optional>
#include <set>
#include <unordered_map>
#include "../VariableTree/VariableTree.h"
#include "PointLogStorage.h"

/*******************************************************************************************************
Start pattern can only define up to minute, when hour is defined then minute should be defined too.
available interval:
1min
5min
30min
1hour
3hour
6hour
12hour
1day
Other values would round down to closest valid interval, anything smaller than 10 would become 10.
********************************************************************************************************/
/********************************************************************************************************
 * PointLogger has a timer and a timer listener.
 * Each time timer listener catch an event, it informs master.
 * Master use the information from the token
 *******************************************************************************************************/
class PointLogger
{
    friend class TimerListener;
public:
    struct StartPattern
    {
        std::optional<uint8_t> hour;
        std::optional<uint8_t> minute;
    };
    PointLogger(std::shared_ptr<VariableTree> _root, PointLogStorage& _storage);
    ~PointLogger();

    void add_point(uint16_t device, uint16_t point, uint32_t secInterval, const std::vector<StartPattern> &startPatternList);

private:
    void handle_timer_event(time_t eventTime, uint32_t token);

    class TimerListener : public EventTimer::Listener
    {
    public:
        TimerListener(PointLogger &_master) : master(_master) {}
        ~TimerListener() {}
        void catch_time_event(time_t eventTime, uint32_t token);

    private:
        PointLogger &master;
    };
    struct PointData
    {
        uint16_t device;
        uint16_t point;
        uint32_t secInterval;
        std::shared_ptr<VariableTree> valuePoint;
    };
    std::shared_ptr<VariableTree> root;
    PointLogStorage& storage;
    std::shared_ptr<TimerListener> timerListener;
    // The key is fusion of device and point Id, to precvent duplication of schedule.
    std::unordered_map<uint32_t, PointData> pointMap;
};

#endif
