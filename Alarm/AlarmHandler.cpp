#include "AlarmHandler.h"
#include "../Basic/DataConverter.h"
#include "AlarmDefinition.h"
#include <iostream>

using namespace std;

AlarmHandler::AlarmHandler() {

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
}

/*
void AlarmHandler::catch_alarm(const HashKey::EitherKey& equipment, const HashKey::EitherKey& source, const Value& leftValue, 
    const HashKey::EitherKey& right, const Value& rightValue, const std::string& message, 
    std::chrono::time_point<std::chrono::system_clock> theMoment, const AlarmDefinition::Condition& condition) 
{
    cout<<"\nGoing to send alarm\n";
    cout<<"Equipment: "<<equipment.to_string()<<endl;
    cout<<"Left Property: "<<source.to_string()<<endl;
    cout<<"Left Value: "<<leftValue.to_string()<<endl;
    if(!right.is_empty()) {
        cout<<"Right: "<<right.to_string()<<endl;
    }
    else {
        cout<<"Constant"<<endl;
    }
    cout<<"Right Value: "<<rightValue.to_string()<<endl;
    cout<<"Message: "<<message<<endl;
    cout<<"Time: "<<DataConverter::ChronoSystemTime(theMoment).to_string()<<endl;
    cout<<"Condition: "<<condition.to_string()<<endl;
    return;
}
*/