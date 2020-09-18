#include "AlarmHandler.h"
#include "../Basic/DataConverter.h"
#include "AlarmDefinition.h"
#include <iostream>

using namespace std;

AlarmHandler::AlarmHandler(const std::string& serverAddress, uint16_t serverPort, const std::string& dbName) : alarmStorage(dbName){
    udpSocket.set_destination(UdpSocket::to_address(serverAddress, serverPort));
}

AlarmHandler::~AlarmHandler() {

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
    //inform thread to run
}

const uint8_t STATE_SEND_ALARM = 0;
const uint8_t STATE_READ_REPLY = 5;
const uint8_t STATE_LOAD_ALARM = 10;
const uint8_t STATE_WAIT_ALARM = 15;

void AlarmHandler::thred_process(AlarmHandler* me) {
    while(1) {
        switch(me->state) {
            case STATE_SEND_ALARM:
                //If got alarm in list
                    //send alarm
                    //go to read state
                //else go get alarm state
                break;
            case STATE_READ_REPLY:
                //sleep 20ms
                //If server got reply
                    //decode it
                    //If acknowledge is true then remove 1 alarm
                    //goto send state
                //Else if total wait is more than 2 seconds, goto send state
                break;
            case STATE_LOAD_ALARM:
                //Clear all ITC message
                //Load alarm
                //If no alarm then goto wait state
                break;
            case STATE_WAIT_ALARM:
                //wiat for message from ITC
                break;
        }
        //Finish all the messages
        
        //Wait for more messages
    }
}
