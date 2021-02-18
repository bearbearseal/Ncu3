#ifndef _TimeEventGun_H_
#define _TimeEventGun_H_
#include <memory>
#include <thread>
#include <unordered_map>
#include <set>
#include <any>

#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/Basic/HashKey.h"

class TimeEventGun
{
public:
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}

        virtual void catch_time_event(uint8_t hour, uint8_t min, uint8_t sec, uint32_t token) {}
        virtual void catch_last_event(uint32_t token) {}
        virtual void catch_time_changed(uint8_t hour, uint8_t min, uint8_t sec, uint32_t token) {}
    };
    TimeEventGun();
    virtual ~TimeEventGun();

    //Not designed to allow add_event while running
    void add_event(uint8_t hour, uint8_t min, uint8_t sec);
    //Not designed to allow add listener while running
    void add_listener(std::weak_ptr<Listener> listener, uint32_t token = 0);

    void system_time_changed();
    void start();
    void stop();

private:
    enum class Command
    {
        None,
        AddEvent,
        AddListener,
        Start,
        Stop,
        TimeChanged,
        Terminate
    };
    struct EventMessage
    {
        EventMessage() { command = Command::None; }
        Command command;
        std::unordered_map<HashKey::EitherKey, std::any, HashKey::EitherKeyHash> data;
    };
    struct ListenerData
    {
        std::weak_ptr<Listener> listener;
        uint32_t token;
    };
    std::unordered_map<void *, ListenerData> listeners;
    std::set<uint32_t> eventTime;
    std::unique_ptr<std::thread> theProcess;
    bool run;
    std::set<uint32_t>::iterator eventIterator;
    uint32_t lastCheckTime;
    ITC<EventMessage> itc;
    std::unique_ptr<ITC<EventMessage>::FixedSocket> threadSocket;
    std::unique_ptr<ITC<EventMessage>::FixedSocket> eventSocket;

    static void thread_process(TimeEventGun *);

    void handle_add_event(const std::unordered_map<HashKey::EitherKey, std::any, HashKey::EitherKeyHash> &messageData);
    void handle_add_listener(std::unordered_map<HashKey::EitherKey, std::any, HashKey::EitherKeyHash> &messageData);
    void handle_start();
    void handle_stop();
    void handle_time_changed();

    void send_last_event_notification();
    void notify_time_event(uint32_t time);
};

#endif