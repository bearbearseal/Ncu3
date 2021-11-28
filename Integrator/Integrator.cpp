#include "Integrator.h"
#include "../Global/GlobalConstant.h"

using namespace std;

Integrator::Integrator(const ConfigStorage &_configData) : configData(_configData)
{
}

Integrator::~Integrator()
{
}

void Integrator::join_schedule_and_points(ScheduleManager &scheduleManager, shared_ptr<VariableTree> &variableTree)
{
    auto pairData = configData.get_point_schedule_pair();
    for (size_t i = 0; i < pairData.size(); ++i)
    {
        auto equipment = variableTree->get_child(pairData[i].deviceId);
        if (equipment == nullptr)
        {
            printf("Got invalid point in equipment pair.\n");
            continue;
        }
        auto point = equipment->get_child(pairData[i].pointId);
        if (point == nullptr)
        {
            printf("Got invalid point in schedule pair.\n");
            continue;
        }
        if(!scheduleManager.has_schedule(pairData[i].scheduleId))
        {
            printf("Got invalid schedule id in schedule pair.\n");
            continue;
        }
        if (!id2ScheduleListenerMap.count(pairData[i].scheduleId))
        {
            auto theListener = make_shared<ScheduleListener>();
            scheduleManager.schedule_add_listener(pairData[i].scheduleId, theListener);
            id2ScheduleListenerMap.emplace(pairData[i].scheduleId, theListener);
        }
        auto theListener = id2ScheduleListenerMap[pairData[i].scheduleId];
        theListener->add_follower(point);
    }
}

Integrator::ScheduleListener::ScheduleListener()
{

}

Integrator::ScheduleListener::~ScheduleListener()
{

}

void Integrator::ScheduleListener::catch_set_event(const Value &setValue)
{
    for(auto i=followerList.begin(); i!=followerList.end();)
    {
        auto shared = i->lock();
        if(shared == nullptr)
        {
            auto temp = i;
            ++i;
            followerList.erase(temp);
        }
        else
        {
            shared->set_value(setValue, GlobalConstant::WRITE_PRIORITY_Schedule);
        }
    }
}

void Integrator::ScheduleListener::catch_unset_event()
{
    for(auto i=followerList.begin(); i!=followerList.end();)
    {
        auto shared = i->lock();
        if(shared == nullptr)
        {
            auto temp = i;
            ++i;
            followerList.erase(temp);
        }
        else
        {
            shared->unset_value(GlobalConstant::WRITE_PRIORITY_Schedule);
        }
    }
}

void Integrator::ScheduleListener::catch_write_event(const Value &newValue)
{
    for(auto i=followerList.begin(); i!=followerList.end();)
    {
        auto shared = i->lock();
        if(shared == nullptr)
        {
            auto temp = i;
            ++i;
            followerList.erase(temp);
        }
        else
        {
            shared->write_value(newValue, GlobalConstant::WRITE_PRIORITY_Schedule);
        }
    }
}

void Integrator::ScheduleListener::add_follower(std::weak_ptr<VariableTree> follower)
{
    followerList.push_back(follower);
}
