#ifndef _AlarmHandler_H_
#define _AlarmHandler_H_
#include <thread>
#include <list>
#include "AlarmListener.h"
#include "AlarmStorage.h"
#include "AlarmDefinition.h"
#include "../../MyLib/UdpSocket/UdpSocket.h"
#include "../../MyLib/ITC/ITC.h"

//Send UDP packet to server.
class AlarmHandler : public AlarmListener {
public:
    AlarmHandler(const std::string& serverAddress, uint16_t serverPort, const std::string& dbName);
    virtual ~AlarmHandler();
    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);

private:
    uint8_t state = 0;
    UdpSocket udpSocket;
    AlarmStorage alarmStorage;
    ITC<AlarmDefinition::AlarmMessage> itc;
    std::list<AlarmStorage::UnreportedAlarm> unReportedAlarm;

    static void thred_process(AlarmHandler* me);
};

#endif