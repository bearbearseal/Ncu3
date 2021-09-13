#pragma once
#include <mutex>
#include <chrono>
#include <map>
#include <unordered_map>
#include "AlarmDefinition.h"
#include "AlarmListener.h"
#include "../../MyLib/UdpSocket/UdpSocket.h"
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/Sqlite/Sqlite3.h"

class AlarmHandler : public AlarmListener {
public:
    static constexpr char COMMAND_ReportAlarm[] = "ReportAlarm";

    AlarmHandler(const std::string& dbName, const std::string& _host, uint16_t _port);
    ~AlarmHandler();
    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);
    void start();
    void stop();

private:
    std::mutex conditionMutex;
    std::unordered_map<HashKey::EitherKey, std::unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKeyHash>, HashKey::EitherKeyHash> point2ConditionMap;
    Sqlite3 theDb;
    std::map<int64_t, std::string> alarmMessages;
    std::unique_ptr<ITC<std::string>::FixedSocket> itcWriteSocket;
    UdpSocket udpSocket;
    std::string serverAddress;
    uint16_t serverPort;
    
    ITC<std::string> itc;

    struct {
        uint8_t state;
        uint8_t failCount;
        std::chrono::time_point<std::chrono::steady_clock> theMoment;
        std::unique_ptr<ITC<std::string>::FixedSocket> itcReadSocket;
    }threadData;

    std::unique_ptr<std::thread> theThread;
    static void thread_process(AlarmHandler* me);

};