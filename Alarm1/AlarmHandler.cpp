#include "AlarmHandler.h"
#include "../../MyLib/Basic/Helper.hpp"
#include "../../OtherLib/nlohmann/json.hpp"

using namespace std;

const uint8_t STATE_SEND_ALARM = 0;
const uint8_t STATE_READ_REPLY = 10;
const uint8_t STATE_READ_MESSAGE = 20;
const uint8_t STATE_RESOLVE_ADDRESS = 30;

AlarmHandler::AlarmHandler(const std::string& dbName, const std::string& _host, uint16_t _port) : theDb(dbName), udpSocket(false), serverAddress(_host), serverPort(_port) {
    //Load message from database
    auto result = theDb.execute_query("Select Id, Message from Alarm Order By Id ASC");
    for(size_t i=0; i<result->get_row_count(); ++i) {
        int id = result->get_integer(i, 0).second;
        string message = result->get_string(i, 1).second;
        printf("Alarm message: %s\n", message.c_str());
        alarmMessages.emplace(id, move(message));
    }
    printf("Has %lu alarms.\n", alarmMessages.size());
    itcWriteSocket = itc.create_fixed_socket(1, 2);
    threadData.itcReadSocket = itc.create_fixed_socket(2, 1);
}

AlarmHandler::~AlarmHandler() {
    stop();
}

void AlarmHandler::start() {
    threadData.state = STATE_RESOLVE_ADDRESS;
    theThread = make_unique<thread>(thread_process, this);
}

void AlarmHandler::stop() {
    if(theThread == nullptr) {
        return;
    }
    //Send an empty string to break the wait_message
    itcWriteSocket->send_message(string());
    theThread->join();
    theThread.reset();
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
    printf("Condition map changed.\n");
    //send message to 
    nlohmann::json alarmData;
    alarmData["Equipment"] = Helper::hashkey_to_json(alarmMessage.pointId.equipmentId).second;
    alarmData["Property"] = Helper::hashkey_to_json(alarmMessage.pointId.propertyId).second;
    alarmData["Value"] = Helper::value_to_json(alarmMessage.value);
    alarmData["Message"] = alarmMessage.message;
    alarmData["MilliSecTime"] = alarmMessage.milliSecTime;
    alarmData["Type"] = alarmMessage.condition.type_to_string();
    alarmData["Code"] = alarmMessage.condition.code;
    alarmData["RightValue"] = Helper::value_to_json(alarmMessage.rightValue);
    itcWriteSocket->send_message(alarmData.dump());
    printf("Message sent to socket.\n");
}

int store_alarm_to_db(const std::string& message, Sqlite3& theDb) {
    auto result = theDb.execute_query("Insert into Alarm (Message)Values('%s');SELECT Last_Insert_Rowid()", message.c_str());
    if(result->get_row_count()) {
        size_t lastId = result->get_integer(0, 0).second;
        return lastId;
    }
    else {
        return -1;
    }
}

pair<bool, int64_t> get_reply_id(const nlohmann::json& replyJson) {
    if(!replyJson.contains("Command") || !replyJson.contains("Data")) {
        return {false, 0};
    }
    const nlohmann::json& commandJson = replyJson["Command"];
    const nlohmann::json& dataJson = replyJson["Data"];
    if(!commandJson.is_string() || !dataJson.is_object()) {
        return {false, 0};
    }
    if(!dataJson.contains("Id")) {
        return {false, 0};
    }
    const nlohmann::json& idJson = dataJson["Id"];
    if(!idJson.is_number_integer()) {
        return {false, 0};
    }
    if(commandJson.get<string>().compare(AlarmHandler::COMMAND_ReportAlarm)) {
        return {false, 0};
    }
    return {true, idJson.get<int64_t>()};
}

void delete_alarm_from_db(int64_t id, Sqlite3& theDb) {
    theDb.execute_update("Delete from Alarm where Id == %ld", id);
}

void AlarmHandler::thread_process(AlarmHandler* me) {
    while(1) {
        switch(me->threadData.state) {
            case STATE_READ_MESSAGE:
                //read all messages
                while(me->threadData.itcReadSocket->has_message()) {
                    string message = me->threadData.itcReadSocket->get_message().message;
                    if(message.size()) {
                        //store each message to database and to alarm queue
                        int id = store_alarm_to_db(message, me->theDb);
                        me->alarmMessages.emplace(id, message);
                    }
                    //empty string is a quit message
                    else {
                        return;
                    }
                }
                if(me->alarmMessages.size()) {
                    me->threadData.state = STATE_SEND_ALARM;
                }
                else {
                    //wait for message;
                    me->threadData.itcReadSocket->wait_message();
                }
                break;
            case STATE_SEND_ALARM:
                //send alarm message;
                {
                    int64_t id = me->alarmMessages.begin()->first;
                    string rawString = me->alarmMessages.begin()->second;
                    //printf("RawString: %s\n", rawString.c_str());
                    nlohmann::json alarmJson;
                    alarmJson["Command"] = COMMAND_ReportAlarm;
                    nlohmann::json& dataJson = alarmJson["Data"];
                    //dataJson["Id"] = id;
                    //nlohmann::json& messageJson = dataJson["Message"];
                    try {
                        dataJson = nlohmann::json::parse(rawString);
                        dataJson["Id"] = id;
                        printf("MessageJson: %s\n", alarmJson.dump().c_str());
                        //send alarm message
                        me->udpSocket.write(alarmJson.dump());
                        me->threadData.state = STATE_READ_REPLY;
                        me->threadData.theMoment = chrono::steady_clock::now();
                    } catch (nlohmann::json::parse_error& jsonError) {
                        //delete message from db
                        delete_alarm_from_db(id, me->theDb);
                        //remove from message
                        me->alarmMessages.erase(id);
                        me->threadData.state = STATE_READ_MESSAGE;
                    }
                }
                break;
            case STATE_READ_REPLY:
                {
                    this_thread::sleep_for(20ms);
                    //if got reply
                    string reply = me->udpSocket.read();
                    if(reply.size()) {
                        printf("Got something: %s.\n", reply.c_str());
                        try {
                            nlohmann::json replyJson = nlohmann::json::parse(reply);
                            auto idPair = get_reply_id(replyJson);
                            printf("got id: %s, %ld\n", idPair.first ? "yes" : "no", idPair.second);
                            if(idPair.first) {
                                me->threadData.state = 0;
                                //remove from database
                                delete_alarm_from_db(idPair.second, me->theDb);
                                //remove message
                                me->alarmMessages.erase(idPair.second);
                                me->threadData.state = STATE_READ_MESSAGE;
                            }
                        } catch(nlohmann::json::parse_error& jsonError) {
                            me->threadData.state = STATE_READ_MESSAGE;
                        }
                    }
                    else if(chrono::steady_clock::now() - me->threadData.theMoment > chrono::seconds(5)) {
                        printf("No reply for 5 second.\n");
                        ++(me->threadData.failCount);
                        if(me->threadData.failCount >= 5) {
                            me->threadData.state = 0;
                            me->threadData.state = STATE_RESOLVE_ADDRESS;
                        }
                        else{
                            me->threadData.state = STATE_READ_MESSAGE;
                        }
                    }
                }
                break;

            case STATE_RESOLVE_ADDRESS:
                printf("Resolving address.\n");
                me->udpSocket.set_destination(UdpSocket::to_address(me->serverAddress, me->serverPort));
                me->threadData.state = STATE_READ_MESSAGE;
                break;
        }
    }
}