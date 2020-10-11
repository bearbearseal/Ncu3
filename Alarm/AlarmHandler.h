#pragma once
#include <mutex>
#include <unordered_map>
#include "AlarmDefinition.h"
#include "AlarmListener.h"
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/Sqlite/Sqlite3.h"
//#include "../../MyLib/UdpSocket/UdpSocket.h"

class AlarmHandler : public AlarmListener{
public:
    AlarmHandler(const std::string& dbName);
    ~AlarmHandler();
    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);

private:
    std::mutex conditionMutex;
    std::unordered_map<HashKey::EitherKey, std::unordered_map<HashKey::EitherKey, AlarmDefinition::Condition, HashKey::EitherKey>, HashKey::EitherKey> point2ConditionMap;
    std::mutex dbMutex;
    Sqlite3 theDb;
    //UdpSocket udpSocket;
};