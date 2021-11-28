#ifndef _Integrator_H_
#define _Integrator_H_
#include "../Storage/ConfigStorage.h"
#include "../Schedule/ScheduleManager.h"
#include "../VariableTree/VariableTree.h"

class Integrator
{
public:
    Integrator(const ConfigStorage &_configData);
    ~Integrator();
    void join_schedule_and_points(ScheduleManager &scheduleManager, std::shared_ptr<VariableTree> &variableTree);

private:
    const ConfigStorage &configData;

    class ScheduleListener : public Schedule::Listener
    {
    public:
        ScheduleListener();
        ~ScheduleListener();
        void catch_set_event(const Value &setValue);
        void catch_unset_event();
        void catch_write_event(const Value &setValue);
        void add_follower(std::weak_ptr<VariableTree> follower);

    private:
        std::vector<std::weak_ptr<VariableTree>> followerList;
    };
    std::unordered_map<uint32_t, std::shared_ptr<ScheduleListener>> id2ScheduleListenerMap;
};

#endif