#ifndef _AlarmVerifyStorePost_H_
#define _AlarmVerifyStorePost_H_
#include <string>
#include <thread>
#include <variant>
//#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/ServerClientITC/ServerClientItc.h"
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/Basic/Value.h"
#include "AlarmDefinition.h"
#include "AlarmProcessor.h"
#include "AlarmStore.h"
#include "AlarmListener.h"

class AlarmVerifyStorePost : public AlarmProcessor::ProcessedStateListener, public AlarmStore
{
public:
    AlarmVerifyStorePost(const std::string &dbFile);
    virtual ~AlarmVerifyStorePost();
    void catch_alarm_state(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                           const Value &refValue, AlarmDefinition::Comparison compare, const std::chrono::time_point<std::chrono::system_clock> theMoment);
    void add_alarm_listener(std::weak_ptr<AlarmListener> listener); //No concurrent protection
    ReadAlarmData read_active_alarm();
    ReadAlarmData read_active_alarm(uint64_t id);
    ReadAlarmData read_history_alarm_id(uint64_t id, size_t count);
    ReadAlarmData read_history_alarm_count(uint64_t beginTime, size_t count);
    ReadAlarmData read_history_alarm_interval(uint64_t beginTime, uint64_t endTime);

    void start();
    void stop();

private:
    enum class Command
    {
        StoreAlarm,
        Stop,
        GetActiveAlarm,
        GetActiveAlarmId,
        GetHistoryAlarmId,
        GetHistoryAlarmCount,
        GetHistoryAlarmInterval
    };
    struct Message2Server
    {
        Message2Server(Command _command, uint64_t param1 = 0, uint64_t param2 = 0) : command(_command), parameter1(param1), parameter2(param2) {}
        Message2Server(Command _command, std::unique_ptr<AlarmDefinition::AlarmMessage> &_alarmMessage) : command(_command), parameter1(0), parameter2(0)
        {
            alarmMessage = std::move(_alarmMessage);
        }
        Message2Server(const Message2Server &theOther)
        {
            command = theOther.command;
            parameter1 = theOther.parameter1;
            parameter2 = theOther.parameter2;
            if (theOther.alarmMessage != nullptr)
                alarmMessage = std::make_unique<AlarmDefinition::AlarmMessage>(*theOther.alarmMessage);
        }
        Message2Server(Message2Server &theOther)
        {
            command = theOther.command;
            alarmMessage = std::move(theOther.alarmMessage);
        }
        Command command;
        uint64_t parameter1;
        uint64_t parameter2;
        std::unique_ptr<AlarmDefinition::AlarmMessage> alarmMessage = nullptr; // unique_ptr is hard to copy
    };
    struct Message2Client
    {
        Command command;
        ReadAlarmData alarms;
    };
    struct ActiveAlarmData
    {
        uint64_t alarmId;
        std::unique_ptr<AlarmDefinition::AlarmMessage> message;
    };

    inline void store_alarm(std::unique_ptr<AlarmDefinition::AlarmMessage> &message);
    std::optional<uint64_t> store_to_active_alarm(std::unique_ptr<AlarmDefinition::AlarmMessage> &message);
    bool remove_from_active_alarm(uint64_t id);
    void store_to_history_alarm(uint64_t id, std::unique_ptr<AlarmDefinition::AlarmMessage> &message);
    void handle_alarm(std::unique_ptr<AlarmDefinition::AlarmMessage> &message);
    ReadAlarmData load_active_alarm();
    ReadAlarmData load_active_alarm(uint64_t id);
    ReadAlarmData load_history_alarm_id(uint64_t id, size_t count);
    ReadAlarmData load_history_alarm_count(uint64_t beginTime, size_t count);
    ReadAlarmData load_history_alarm_interval(uint64_t beginTime, uint64_t endTime);
    static void the_process(AlarmVerifyStorePost *me);

    std::unordered_map<void*, std::weak_ptr<AlarmListener>> listenerMap;

    //ITC<std::variant<Message2Thread, Message2Sender>> itc;
    //std::unique_ptr<ITC<std::variant<Message2Thread, Message2Sender>>::FixedSocket> threadSocket;
    //std::unique_ptr<ITC<std::variant<Message2Thread, Message2Sender>>::FixedSocket> senderSocket;
    ServerClientItc<Message2Server, Message2Client> itc;
    std::unique_ptr<ServerClientItc<Message2Server, Message2Client>::ServerSocket> serverSocket;
    // threadSocket and senderSocket took 1 and 2, nextSocket would take 3 onwards.
    size_t nextSocketId = 3;

    std::unordered_map<uint64_t, ActiveAlarmData> point2ActiveAlarmMap;

    Sqlite3 theDb;
    std::unique_ptr<std::thread> theProcess;
    std::unordered_map<uint64_t, uint64_t> pointId2alarmIdMap;
};

#endif