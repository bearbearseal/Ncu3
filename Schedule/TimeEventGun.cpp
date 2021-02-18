#include "TimeEventGun.h"
#include <iostream>

using namespace std;

struct HourMinuteSecond
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

HourMinuteSecond chrono_time_to_hms(const chrono::time_point<chrono::system_clock> &theTime)
{
    time_t totalSec = chrono::system_clock::to_time_t(theTime);
    tm theTm;
    localtime_r(&totalSec, &theTm);
    return {uint8_t(theTm.tm_hour), uint8_t(theTm.tm_min), uint8_t(theTm.tm_sec)};
    /*
    uint64_t totalSecond = chrono::duration_cast<chrono::seconds>(theTime.time_since_epoch()).count();
    uint8_t second = uint8_t(totalSecond % 60);
    uint8_t minute = uint8_t((totalSecond / 60) % 60);
    uint8_t hour = uint8_t((totalSecond / 3600) % 24);
    return {hour, minute, second};
    */
}

uint32_t chrono_time_to_day_second(const chrono::time_point<chrono::system_clock> &theTime)
{
    time_t totalSec = chrono::system_clock::to_time_t(theTime);
    tm theTm;
    localtime_r(&totalSec, &theTm);
    uint32_t retVal = theTm.tm_hour * 3600 + theTm.tm_min * 60 + theTm.tm_sec;
    return retVal;
    /*
    uint64_t totalSecond = chrono::duration_cast<chrono::seconds>(theTime.time_since_epoch()).count();
    return uint32_t(totalSecond % (24 * 3600));
    */
}

HourMinuteSecond day_time_to_hms(uint32_t theTime)
{
    printf("Time %u to hms.\n", theTime);
    uint8_t second = uint8_t(theTime % 60);
    uint8_t minute = uint8_t((theTime / 60) % 60);
    uint8_t hour = uint8_t((theTime / 3600) % 24);
    printf("Time %u %u %u to hms.\n", hour, minute, second);
    return {hour, minute, second};
}

TimeEventGun::TimeEventGun()
{
    threadSocket = itc.create_fixed_socket(1, 2);
    eventSocket = itc.create_fixed_socket(2, 1);
    theProcess = make_unique<thread>(thread_process, this);
    run = false;
}

TimeEventGun::~TimeEventGun()
{
    EventMessage message;
    message.command = Command::Terminate;
    eventSocket->send_message(message);
    theProcess->join();
}

void TimeEventGun::add_event(uint8_t hour, uint8_t minute, uint8_t sec)
{
    EventMessage message;
    message.command = Command::AddEvent;
    message.data.emplace("Hour", hour);
    message.data.emplace("Minute", minute);
    message.data.emplace("Second", sec);
    eventSocket->send_message(message);
}

void TimeEventGun::add_listener(std::weak_ptr<Listener> listener, uint32_t token)
{
    EventMessage message;
    message.command = Command::AddListener;
    message.data.emplace("Listener", listener);
    message.data.emplace("Token", token);
    eventSocket->send_message(message);
}

void TimeEventGun::system_time_changed()
{
    EventMessage message;
    message.command = Command::TimeChanged;
    eventSocket->send_message(message);
}

void TimeEventGun::start()
{
    EventMessage message;
    message.command = Command::Start;
    eventSocket->send_message(message);
}

void TimeEventGun::stop()
{
    EventMessage message;
    message.command = Command::Stop;
    eventSocket->send_message(message);
}

void TimeEventGun::handle_add_event(const unordered_map<HashKey::EitherKey, any, HashKey::EitherKeyHash> &messageData)
{
    if (messageData.count("Hour") && messageData.count("Minute") && messageData.count("Second"))
    {
        try
        {
            uint8_t hour = any_cast<uint8_t>(messageData.at("Hour"));
            uint8_t minute = any_cast<uint8_t>(messageData.at("Minute"));
            uint8_t second = any_cast<uint8_t>(messageData.at("Second"));
            uint32_t theTime = uint32_t(hour) * 3600 + uint32_t(minute) * 60 + uint32_t(second);
            eventTime.emplace(theTime);
        }
        catch (const std::bad_any_cast &e)
        {
            std::cout << e.what() << '\n';
        }
    }
}

void TimeEventGun::handle_add_listener(unordered_map<HashKey::EitherKey, any, HashKey::EitherKeyHash> &messageData)
{
    if (messageData.count("Listener") && messageData.count("Token"))
    {
        try
        {
            weak_ptr<TimeEventGun::Listener> listener = any_cast<weak_ptr<TimeEventGun::Listener>>(messageData.at("Listener"));
            uint32_t token = any_cast<uint32_t>(messageData.at("Token"));
            auto shared = listener.lock();
            if (shared != nullptr)
            {
                ListenerData entry;
                entry.listener = listener;
                entry.token = token;
                listeners.emplace(shared.get(), entry);
            }
        }
        catch (const bad_any_cast &e)
        {
            cout << e.what() << '\n';
        }
    }
}

void TimeEventGun::handle_start()
{
    if (!run)
    {
        run = true;
        uint32_t thisMoment = chrono_time_to_day_second(chrono::system_clock::now());
        eventIterator = eventTime.upper_bound(thisMoment);
        lastCheckTime = thisMoment;
    }
}

void TimeEventGun::handle_stop()
{
    run = false;
    eventIterator = eventTime.end();
}

void TimeEventGun::handle_time_changed()
{
    lastCheckTime = chrono_time_to_day_second(chrono::system_clock::now());
    eventIterator = eventTime.upper_bound(lastCheckTime);
    for (auto i = listeners.begin(); i != listeners.end();)
    {
        auto shared = i->second.listener.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listeners.erase(temp);
        }
        else
        {
            HourMinuteSecond hms = day_time_to_hms(lastCheckTime);
            shared->catch_time_changed(hms.hour, hms.minute, hms.second, i->second.token);
            ++i;
        }
    }
}

void TimeEventGun::send_last_event_notification()
{
    for (auto i = listeners.begin(); i != listeners.end();)
    {
        auto shared = i->second.listener.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listeners.erase(temp);
        }
        else
        {
            shared->catch_last_event(i->second.token);
            ++i;
        }
    }
}

void TimeEventGun::notify_time_event(uint32_t time)
{
    for (auto i = listeners.begin(); i != listeners.end();)
    {
        auto shared = i->second.listener.lock();
        if (shared == nullptr)
        {
            auto temp = i;
            ++i;
            listeners.erase(temp);
        }
        else
        {
            HourMinuteSecond hms = day_time_to_hms(time);
            shared->catch_time_event(hms.hour, hms.minute, hms.second, i->second.token);
            ++i;
        }
    }
}

void TimeEventGun::thread_process(TimeEventGun *me)
{
    while (1)
    {
        while (me->threadSocket->has_message())
        {
            //Process message
            auto message = me->threadSocket->get_message();
            switch (message.message.command)
            {
            case Command::AddEvent:
                me->handle_add_event(message.message.data);
                break;
            case Command::AddListener:
                me->handle_add_listener(message.message.data);
                break;
            case Command::Start:
                me->handle_start();
                break;
            case Command::Stop:
                me->handle_stop();
                break;
            case Command::TimeChanged:
                me->handle_time_changed();
                break;
            case Command::Terminate:
                return;
            default:
                break;
            }
        }
        if (me->run)
        {
            //Handle exception where no event time is present
            if (!me->eventTime.size())
            {
                //get message to c if got any change of state
                if (!me->threadSocket->wait_message())
                {
                    //itc is gone, end the thread
                    return;
                }
                continue;
            }
            //Handle exception where the previously handled time event was the last event of the day
            if (me->eventIterator == me->eventTime.end())
            {
                me->send_last_event_notification();
                me->eventIterator = me->eventTime.begin();
            }
            uint32_t nextEventDelay = *(me->eventIterator);
            if (me->lastCheckTime >= nextEventDelay && me->eventIterator == me->eventTime.begin())
            {
                //Event should happen tomorrow.
                nextEventDelay += (24 * 3600);
                nextEventDelay -= chrono_time_to_day_second(chrono::system_clock::now());
            }
            else {
                nextEventDelay -= me->lastCheckTime;
            }
            chrono::seconds duration(nextEventDelay);
            auto result = me->threadSocket->wait_message(duration);
            if (!result.first)
            {
                //itc is gone, end the thread
                return;
            }
            me->lastCheckTime = chrono_time_to_day_second(chrono::system_clock::now());
            if (result.second)
            {
                //Interrupted by message
            }
            else
            {
                //timeout, send notification
                me->notify_time_event(me->lastCheckTime);
                ++me->eventIterator;
            }
        }
        //Thread is not started, wait till got a start message
        else if (!me->threadSocket->wait_message())
        {
            //itc is gone liao, end the thread
            return;
        }
    }
}
