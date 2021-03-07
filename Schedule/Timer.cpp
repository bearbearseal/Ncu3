#include "Timer.h"

using namespace std;

Timer::Timer()
{
    eventSocket = itc.create_fixed_socket(1, 2);
    threadSocket = itc.create_fixed_socket(2, 1);
    theProcess = make_unique<thread>(thread_process, this);
}

Timer::~Timer()
{
}

void Timer::terminate()
{
    Message message;
    message.command = Command::Terminate;
    eventSocket->send_message(message);
}

void Timer::time_changed()
{
    Message message;
    message.command = Command::TimeChanged;
    eventSocket->send_message(message);
}

void Timer::add_time_event(time_t eventTime, std::weak_ptr<Listener> listener, uint16_t token)
{
    Message message;
    message.command = Command::AddEvent;
    message.eventTime = eventTime;
    message.listener = listener;
    message.token = token;
    printf("Sending add event message.\n");
    eventSocket->send_message(message);
}

void Timer::handle_add_event(const Message &message)
{
    auto &entry = second2ListenerListMap[message.eventTime];
    entry.push_back({message.listener, message.token});
}

void Timer::thread_process(Timer *me)
{
    while (1)
    {
        //Handle each message
        while (me->threadSocket->has_message())
        {
            auto message = me->threadSocket->get_message();
            switch (message.message.command)
            {
            case Command::AddEvent:
                me->second2ListenerListMap[message.message.eventTime].push_back({message.message.listener, message.message.token});
                printf("Adding listener to %ld\n", message.message.eventTime);
                break;
            case Command::TimeChanged:
                break;
            case Command::Terminate:
            default:
                return;
            }
        }
        time_t secondNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
        //handle all events that should had been called
        auto firstEntry = me->second2ListenerListMap.begin();
        while (firstEntry != me->second2ListenerListMap.end())
        {
            time_t eventSeconds = firstEntry->first;
            //inform everyone and go to next one
            if (eventSeconds <= secondNow)
            {
                for (auto i = firstEntry->second.begin(); i != firstEntry->second.end(); ++i)
                {
                    auto shared = i->listener.lock();
                    if (shared != nullptr)
                    {
                        shared->catch_time_event(eventSeconds, i->token);
                    }
                }
                printf("Removing entry %ld\n", firstEntry->first);
                //then remove this event
                me->second2ListenerListMap.erase(firstEntry);
                firstEntry = me->second2ListenerListMap.begin();
            }
            else
            {
                printf("Would wait %ld seconds for message.\n", eventSeconds - secondNow);
                me->threadSocket->wait_message(chrono::seconds(eventSeconds - secondNow));
                break;
            }
        }
    }
}
