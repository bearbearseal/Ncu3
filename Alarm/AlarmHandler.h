#ifndef _AlarmHandler_H_
#define _AlarmHandler_H_
#include <thread>
#include <list>
#include <mutex>
#include <memory>
#include "AlarmListener.h"
#include "AlarmStorage.h"
#include "AlarmDefinition.h"
#include "../../MyLib/UdpSocket/UdpSocket.h"
#include "../../MyLib/ITC/ITC.h"

//Send UDP packet to server.
class AlarmHandler : public AlarmListener {
public:
    //AlarmHandler(const std::string& serverAddress, uint16_t serverPort, const std::string& dbName);
    AlarmHandler(const std::string& serverAddress, uint16_t serverPort, std::unique_ptr<AlarmStorage>& alarmStorage);
    virtual ~AlarmHandler();
    virtual void catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);

private:
    ITC<bool> itc;
    //Thread data {
    std::thread* theProcess = nullptr;
    bool threadRun = true;
    uint8_t state = 0;
    std::chrono::time_point<std::chrono::steady_clock> timeRecorder;
    UdpSocket udpSocket;
    std::shared_ptr<ITC<bool>::FixedSocket> threadSocket;
    //}
    std::shared_ptr<ITC<bool>::FixedSocket> messageSocket;
    std::mutex storageMutex;
    //AlarmStorage alarmStorage;
    std::unique_ptr<AlarmStorage> alarmStorage;
    std::list<AlarmStorage::UnreportedAlarm> unReportedAlarm;

    static void thread_process(AlarmHandler* me);
};

#endif