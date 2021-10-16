#ifndef _AlarmStore_H_
#define _AlarmStore_H_
#include "AlarmManager.h"
#include "../../MyLib/ITC/ITC.h"

class AlarmPoster : public AlarmManager::AlarmListener
{
public:
    AlarmPoster();
    virtual ~AlarmPoster();

    void catch_alarm(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                     const Value &refValue, AlarmDefinition::Comparison compare, const std::chrono::time_point<std::chrono::system_clock> theMoment);
    void start();
    void stop();

private:
    static void thread_process(void *threadData);

    struct PointData
    {
        AlarmDefinition::AlarmState state;
        std::chrono::time_point<std::chrono::system_clock> theMoment;
    };
    std::unordered_map<uint64_t, PointData> id2PointDataMap;
    struct Message
    {
        uint64_t pointId;
        Value theValue;
        AlarmDefinition::AlarmState state;
        Value refValue;
        AlarmDefinition::Comparison compare;
        std::chrono::time_point<std::chrono::system_clock> theMoment;
    };
    ITC<Message> itc;
    std::unique_ptr<ITC<Message>::FixedSocket> threadSocket;
    std::unique_ptr<ITC<Message>::FixedSocket> masterSocket;
    std::unique_ptr<std::thread> theProcess;
};

#endif