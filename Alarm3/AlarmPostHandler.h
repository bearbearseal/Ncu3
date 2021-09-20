#ifndef _AlarmPostage_H_
#define _AlarmPostage_H_
#include "AlarmDefinition.h"

class AlarmPostHandler
{
public:
    AlarmPostHandler() {}
    virtual ~AlarmPostHandler() {}

    void pickup_alarm(AlarmDefinition::AlarmMessage &&alarm);
    void pickup_alarm(const AlarmDefinition::AlarmMessage &alarm);

    class WareHouse
    {
    public:
        WareHouse(){}
        virtual ~WareHouse() {}
        virtual void store(AlarmDefinition::AlarmMessage) = 0;
        virtual std::pair<uint32_t, AlarmDefinition::AlarmMessage> load_next() = 0;
        virtual void delete_next() = 0;
        //Load active alarm could be added in future
    };
    class Deliveryman
    {
        Deliveryman() {}
        virtual ~Deliveryman() {}
        virtual bool deliver_alarm(uint32_t alarmId, AlarmDefinition::AlarmMessage) = 0;
    };
};

#endif