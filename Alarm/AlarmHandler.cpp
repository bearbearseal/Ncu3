#include "AlarmHandler.h"
#include "../Basic/DataConverter.h"
#include "AlarmDefinition.h"
#include "../../OtherLib/nlohmann/json.hpp"
#include <iostream>

using namespace std;

const uint8_t STATE_SEND_ALARM = 0;
const uint8_t STATE_READ_REPLY = 5;
const uint8_t STATE_LOAD_ALARM = 10;
//const uint8_t STATE_WAIT_ALARM = 15;

AlarmHandler::AlarmHandler(const std::string& serverAddress, uint16_t serverPort, const std::string& dbName) : alarmStorage(dbName){
    udpSocket.set_destination(UdpSocket::to_address(serverAddress, serverPort));
    threadSocket = itc.create_fixed_socket(1, 2);
    messageSocket = itc.create_fixed_socket(2, 1);
    state = STATE_LOAD_ALARM;
    threadRun = true;
    theProcess = new thread(thread_process, this);
    cout<<"Alarm handler starts.\n";
}

AlarmHandler::~AlarmHandler() {
    threadRun = false;
    messageSocket->send_message(threadRun);
    theProcess->join();
    delete theProcess;
    theProcess = nullptr;
}

void AlarmHandler::catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage) {
    cout<<"Equipment: "<<alarmMessage.equipment.to_string()<<endl;
    cout<<"Left Property: "<<alarmMessage.source.to_string()<<endl;
    cout<<"Left Value: "<<alarmMessage.leftValue.to_string()<<endl;
    if(!alarmMessage.right.is_empty()) {
        cout<<"Right: "<<alarmMessage.right.to_string()<<endl;
    }
    else {
        cout<<"Constant"<<endl;
    }
    cout<<"Right Value: "<<alarmMessage.rightValue.to_string()<<endl;
    cout<<"Message: "<<alarmMessage.message<<endl;
    cout<<"Time: "<<DataConverter::ChronoSystemTime(alarmMessage.theMoment).to_string()<<endl;
    cout<<"Condition: "<<alarmMessage.condition.to_string()<<endl;

    //Store to table
    {
        lock_guard<mutex> lock(storageMutex);
        alarmStorage.store_alarm(alarmMessage);
    }
    //inform thread to run
    bool message = true;
    messageSocket->send_message(message);
}

bool reply_is_valid(const nlohmann::json& theReply, size_t expectedId) {
    if(!theReply.contains("Command") || !theReply.contains("Id")) {
        return false;
    }
    const nlohmann::json& command = theReply["Command"];
    const nlohmann::json& id = theReply["Id"];
    if(!command.is_string() || !id.is_number_integer()) {
        return false;
    }
    if(command.get<string>().compare("AlarmAck")) {
        return false;
    }
    if(id.get<size_t>() != expectedId) {
        return false;
    }
    return true;
}

void AlarmHandler::thread_process(AlarmHandler* me) {
    while(1) {
        switch(me->state) {
            case STATE_SEND_ALARM:
                //If got alarm in list
                if(me->unReportedAlarm.size()) {
                    //send alarm
	                nlohmann::json jsonAlarm;
                    AlarmStorage::UnreportedAlarm& head = me->unReportedAlarm.front();
                    jsonAlarm["Command"] = "Alarm";
                    nlohmann::json& message = jsonAlarm["Message"];
                    message["Id"] = head.id;
                    message["PriorId"] = head.priorId;
                    message["Equipment"] = head.equipment;
                    message["Property"] = head.property;
                    message["Message"] = head.message;
                    message["Value"] = head.value;
                    message["TimeMilliSec"] = head.timeMilliSec;
                    message["Type"] = head.type;
                    message["Code"] = head.code;
                    me->udpSocket.write(jsonAlarm.dump());
                    me->timeRecorder = chrono::steady_clock::now();
                    //go to read state
                }
                //else go get alarm state
                else {
                    me->state = STATE_LOAD_ALARM;
                }
                break;
            case STATE_READ_REPLY:
                //sleep 20ms
                this_thread::sleep_for(20ms);
                {
                    //If server got reply
                    string serverReply = me->udpSocket.read();
                    nlohmann::json replyJson;
                    if(serverReply.size()) {
                        //decode it
                        try {
                            //Reply format {"Command":"AlarmAck", "Id": <id>}
                            replyJson = nlohmann::json::parse(serverReply);
                            //If acknowledge is true then remove 1 alarm
                            if(reply_is_valid(replyJson, me->unReportedAlarm.front().id)) {
                                me->unReportedAlarm.pop_front();
                                //goto send state
                                me->state = STATE_SEND_ALARM;
                            }
                        }
                        catch (nlohmann::json::parse_error)
                        {
                        }
                    }
                    //Else if total wait is more than 2 seconds, goto send state
                    else if(chrono::steady_clock::now() - me->timeRecorder > chrono::seconds(2)) {
                        me->state = STATE_SEND_ALARM;
                    }
                }
                break;
            case STATE_LOAD_ALARM:
                //Clear all ITC message
                while(me->threadSocket->has_message()) {
                    me->threadSocket->get_message();
                }
                //Load alarm
                {
                    lock_guard<mutex> lock(me->storageMutex);
                    me->unReportedAlarm = me->alarmStorage.get_unreported_alarms(20);
                }
                //If no alarm then goto wait state
                if(me->unReportedAlarm.size()) {
                    me->state = STATE_SEND_ALARM;
                }
                else {
                    me->threadSocket->wait_message();
                }
                break;
            default:
                me->state = STATE_SEND_ALARM;
                break;
        }
        //Finish all the messages
        
        //Wait for more messages
    }
}
