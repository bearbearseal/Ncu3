#ifndef _Timer_H_
#define _Timer_H_
#include <memory>
#include <map>
#include <vector>
#include <thread>
#include "../../MyLib/ITC/ITC.h"

class Timer {
public:
    class Listener {
    public:
        Listener() {}
        virtual ~Listener() {}

        virtual void catch_time_event(time_t eventTime, uint32_t token) {}

    };
    Timer();
    virtual ~Timer();

    void terminate();
    void time_changed();
    void add_time_event(time_t eventTime, std::weak_ptr<Listener> listener, uint16_t token);

private:
    enum class Command {
        AddEvent,
        TimeChanged,
        Terminate
    };
    struct ListenerData {
        std::weak_ptr<Listener> listener;
        uint16_t token;
    };
    std::map<time_t, std::vector<ListenerData>> second2ListenerListMap;

    struct Message {
        Command command;
        time_t eventTime;
        std::weak_ptr<Listener> listener;
        uint16_t token;
    };

    std::unique_ptr<std::thread> theProcess;
    ITC<Message> itc;
    std::unique_ptr<ITC<Message>::FixedSocket> eventSocket;
    std::unique_ptr<ITC<Message>::FixedSocket> threadSocket;
    size_t processState = 0;

    static void thread_process(Timer* me);
    void handle_add_event(const Message& message);
};

#endif