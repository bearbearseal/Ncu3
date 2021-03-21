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
        switch (me->processState)
        {
        case 0: //Initial State
            me->threadSocket->wait_message();
            me->processState = 10;
            break;

        case 10: //Read message
        {
            //time_t secondNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
            while (me->threadSocket->has_message())
            {
                auto message = me->threadSocket->get_message();
                switch (message.message.command)
                {
                case Command::AddEvent:
                    printf("Adding event at %ld\n", message.message.eventTime);
                    me->second2ListenerListMap[message.message.eventTime].push_back({message.message.listener, message.message.token});
                    break;
                case Command::TimeChanged:
                    break;
                case Command::Terminate:
                default:
                    return;
                }
            }
            printf("No more message.\n");
            me->processState = 20;
            break;
        }
        case 20: //Wait for event
        {
            printf("Wait for event state.\n");
            auto firstEntry = me->second2ListenerListMap.begin();
            if (firstEntry == me->second2ListenerListMap.end())
            {
                me->processState = 0; //wait for message
            }
            else
            {
                time_t secondNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
                time_t eventSecond = firstEntry->first;
                if(eventSecond <= secondNow)
                {
                    for (auto i = firstEntry->second.begin(); i != firstEntry->second.end(); ++i)
                    {
                        auto shared = i->listener.lock();
                        if (shared != nullptr)
                        {
                            shared->catch_time_event(eventSecond, i->token);
                        }
                    }
                    me->second2ListenerListMap.erase(firstEntry);
                }
                else
                {
                    printf("Going to wait %lu seconds for event.\n", eventSecond - secondNow);
                    me->threadSocket->wait_message(chrono::seconds(eventSecond - secondNow));
                    if(me->threadSocket->has_message())
                    {
                        me->processState = 10;   //Get message
                    }
                }
            }
            break;
        }
        }
    }
}
