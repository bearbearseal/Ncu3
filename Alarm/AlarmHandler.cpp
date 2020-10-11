#include "AlarmHandler.h"
#include "../../MyLib/Basic/Helper.hpp"
#include "../../OtherLib/nlohmann/json.hpp"

using namespace std;

AlarmHandler::AlarmHandler(const std::string& dbName) : theDb(dbName) {

}

AlarmHandler::~AlarmHandler() {

}

void AlarmHandler::catch_alarm(const AlarmDefinition::AlarmMessage& alarmMessage) {
    printf("Got alarm.\n");
    printf("EquipmentId: %s\nProperty:%s\n", alarmMessage.pointId.equipmentId.to_string().c_str(), alarmMessage.pointId.propertyId.to_string().c_str());
    printf("Message: %s\n", alarmMessage.message.c_str());
    {
        lock_guard<mutex> lock(conditionMutex);
        auto i = point2ConditionMap.find(alarmMessage.pointId.equipmentId);
        if(i != point2ConditionMap.end()) {
            auto j = i->second.find(alarmMessage.pointId.propertyId);
            if(j != i->second.end()) {
                if(alarmMessage.condition == j->second) {
                    //smae condition, no need to process
                    printf("Ignored.\n");
                    return;
                }
            }
        }
        point2ConditionMap[alarmMessage.pointId.equipmentId][alarmMessage.pointId.propertyId] = alarmMessage.condition;
    }
    //store to sqlite
    nlohmann::json alarmJson;
    alarmJson["Command"] = "ReportAlarm";
    nlohmann::json& alarmData = alarmJson["Data"];
    alarmData["EquipmentId"] = Helper::hashkey_to_json(alarmMessage.pointId.equipmentId).second;
    alarmData["PropertyId"] = Helper::hashkey_to_json(alarmMessage.pointId.propertyId).second;
    alarmData["Value"] = Helper::value_to_json(alarmMessage.value);
    alarmData["Message"] = alarmMessage.message;
    alarmData["MilliSecTime"] = alarmMessage.milliSecTime;
    alarmData["Condition"] = alarmMessage.condition.type_to_string();
    alarmData["Code"] = alarmMessage.condition.code;
    alarmData["RightValue"] = Helper::value_to_json(alarmMessage.rightValue);
    {
        lock_guard<mutex> lock(dbMutex);
        printf("%s\n", alarmJson.dump().c_str());
        bool success = theDb.execute_update("Insert into Alarm (Message)Values('%s')", alarmJson.dump().c_str());
        printf("%s\n", success ? "good" : "bad");
    }
}
