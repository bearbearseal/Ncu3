#ifndef _AlarmStorage_H_
#define _AlarmStorage_H_
#include <string>
#include <thread>
#include <variant>
#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/Basic/Value.h"
#include "AlarmDefinition.h"

class AlarmStorage
{
public:
    struct AlarmMessage
    {
        uint32_t deviceId;
        uint32_t pointId;
        Value activeValue;
        Value refValue;
        uint16_t comparison;
        uint16_t alarmState;
        uint64_t msTime;
    };
    AlarmStorage(const std::string& dbFile);
    virtual ~AlarmStorage();
    std::vector<std::pair<uint64_t, AlarmMessage>> load_active_alarms();
    std::vector<std::pair<uint64_t, AlarmMessage>> load_history_alarms_begin_time(uint64_t msTime, size_t count);
    std::vector<std::pair<uint64_t, AlarmMessage>> load_history_alarms_interval(uint64_t beginTime, uint64_t endTime);
    std::vector<std::pair<uint64_t, AlarmMessage>> load_history_alarm(uint64_t id, uint16_t count); //if count>100, then count=100
    std::vector<std::pair<uint64_t, AlarmMessage>> load_history_alarm(uint64_t msTimeBegin, uint64_t msTimeEnd); //if count>100, then count=100
    void start();
    void stop();
    void store_alarm(std::unique_ptr<AlarmMessage>& message);

private:
    enum class Command
    {
        StoreAlarm,
        Stop,
        GetActiveAlarm,
        GetHistoryAlarm
    };
    struct Message2Thread
    {
        Command command;
        uint64_t alarmId;
        std::unique_ptr<AlarmMessage> message;
    };
    struct Message2Sender
    {
        Command command;
        std::vector<std::pair<uint64_t, AlarmMessage>> alarms;
    };
    struct ActiveAlarmData
    {
        uint64_t alarmId;
        std::unique_ptr<AlarmMessage> message;
    };

    uint64_t store_to_active_alarm(std::unique_ptr<AlarmMessage>& message);
    bool remove_from_active_alarm(uint64_t id);
    void store_to_history_alarm(uint64_t id, std::unique_ptr<AlarmMessage>& message);
    void handle_alarm(std::unique_ptr<AlarmMessage>& message);
    static void the_process(AlarmStorage* me);

    ITC<std::variant<Message2Thread, Message2Sender>> itc;
    std::unique_ptr<ITC<std::variant<Message2Thread, Message2Sender>>::FixedSocket> threadSocket;
    std::unique_ptr<ITC<std::variant<Message2Thread, Message2Sender>>::FixedSocket> senderSocket;

    std::unordered_map<uint64_t, ActiveAlarmData> point2ActiveAlarmMap;

    Sqlite3 theDb;
    std::unique_ptr<std::thread> theProcess;
    std::unordered_map<uint64_t, uint64_t> pointId2alarmIdMap;
};

#endif