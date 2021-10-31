#include "AlarmTalker.h"
#include <thread>

using namespace std;

const string ALARM_FIELD_Id = "Id";
const string ALARM_FIELD_State = "State";
const string ALARM_FIELD_ActiveValue = "ActiveValue";
const string ALARM_FIELD_RefValue = "RefValue";
const string ALARM_FIELD_Compare = "Compare";
const string ALARM_FIELD_Time_ms = "Time_ms";
const string ALARM_FIELD_Device = "Device";
const string ALARM_FIELD_Point = "Point";

void AlarmTalker::alarm_data_to_json(nlohmann::json &dest, AlarmStore::ReadAlarmData data)
{
    if(data.continuation)
    {
        dest[FIELD_Continuation] = data.continuation;
    }
    nlohmann::json& arrayDest = dest[FIELD_Alarm];
    vector<pair<uint64_t, AlarmDefinition::AlarmMessage>>& alarmList = data.alarmList;
    for(size_t i=0; i<alarmList.size(); ++i)
    {
        nlohmann::json entry;
        entry[ALARM_FIELD_Id] = alarmList[i].first;
        entry[ALARM_FIELD_State] = alarmList[i].second.alarmState;
        entry[ALARM_FIELD_ActiveValue] = alarmList[i].second.activeValue.get_float();
        entry[ALARM_FIELD_RefValue] = alarmList[i].second.refValue.get_float();
        entry[ALARM_FIELD_Compare] = alarmList[i].second.comparison;
        entry[ALARM_FIELD_Time_ms] = alarmList[i].second.msTime;
        entry[ALARM_FIELD_Device] = alarmList[i].second.deviceId;
        entry[ALARM_FIELD_Point] = alarmList[i].second.pointId;
        arrayDest.push_back(move(entry));
    }
}

AlarmTalker::AlarmTalker(uint16_t portNum, shared_ptr<AlarmStore> _alarmStore) : TcpListener(portNum), alarmStore(_alarmStore)
{
    myShadow = make_shared<Shadow>(*this);
}

AlarmTalker::~AlarmTalker()
{
    weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while (weak.lock())
    {
        this_thread::yield();
    }
}

void AlarmTalker::catch_message(std::string &input, size_t handle)
{
    nlohmann::json theJson;
    nlohmann::json theReply;
    try
    {
        theJson = nlohmann::json::parse(input);
    }
    catch (nlohmann::json::parse_error &error)
    {
        printf("Cannot parse message: %s\n", input.c_str());
        theReply["Status"] = "Bad";
        theReply["Message"] = "Parse error";
        this->write_message(handle, theReply.dump() + '\n');
        return;
    }
    if (!theJson.is_object())
    {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Input is not an object";
        this->write_message(handle, theReply.dump() + '\n');
        return;
    }
    auto iSequence = theJson.find(FIELD_Sequence);
    if(iSequence != theJson.end())
    {
        theReply[FIELD_Sequence] = *iSequence;
    }
    auto iCommand = theJson.find(FIELD_Command);
    if (iCommand == theJson.end())
    {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Input does not have command";
        this->write_message(handle, theReply.dump() + '\n');
        return;
    }
    if (iCommand->get<string>() == COMMAND_ReadActiveAlarm)
    {
        AlarmStore::ReadAlarmData result;
        auto iContinuation = theJson.find(FIELD_Continuation);
        if (iContinuation != theJson.end() && iContinuation->is_number_integer())
        {
            result = alarmStore->read_active_alarm(iContinuation->get<uint64_t>());
        }
        else
        {
            result = alarmStore->read_active_alarm();
        }
        alarm_data_to_json(theReply, result);
        this->write_message(handle, theReply.dump() + '\n');
    }
    else if (iCommand->get<string>() == COMMAND_ReadHistoryAlarm)
    {
        AlarmStore::ReadAlarmData result;
        auto iContinuation = theJson.find(FIELD_Continuation);
        if (iContinuation != theJson.end() && iContinuation->is_number_integer())
        {
            auto iCount = theJson.find(FIELD_Count);
            if (iCount != theJson.end() && iCount->is_number_integer())
            {
                result = alarmStore->read_history_alarm_id(iContinuation->get<uint64_t>(), iCount->get<uint64_t>());
            }
            else
            {
                result = alarmStore->read_history_alarm_id(iContinuation->get<uint64_t>());
            }
            alarm_data_to_json(theReply, result);
            this->write_message(handle, theReply.dump() + '\n');
        }
        else
        {
            auto iBegin = theJson.find(FIELD_Begin);
            if (iBegin != theJson.end() && iBegin->is_number_integer())
            {
                auto iCount = theJson.find(FIELD_Count);
                if (iCount != theJson.end() && iCount->is_number_integer())
                {
                    result = alarmStore->read_history_alarm_count(iBegin->get<uint64_t>(), iCount->get<uint64_t>());
                    alarm_data_to_json(theReply, result);
                    this->write_message(handle, theReply.dump() + '\n');
                }
                else
                {
                    auto iEnd = theJson.find(FIELD_End);
                    if (iEnd != theJson.end() && iEnd->is_number_integer())
                    {
                        result = alarmStore->read_history_alarm_interval(iBegin->get<uint64_t>(), iEnd->get<uint64_t>());
                        alarm_data_to_json(theReply, result);
                        this->write_message(handle, theReply.dump() + '\n');
                    }
                    else
                    {
                        // Cannot process
                        theReply["Status"] = "Bad";
                        theReply["Message"] = "Missing parameters in ReadHistoryAlarm Command";
                        this->write_message(handle, theReply.dump() + '\n');
                    }
                }
            }
        }
    }
    else
    {
        theReply["Status"] = "Bad";
        theReply["Message"] = "Unknown Command";
        this->write_message(handle, theReply.dump() + '\n');
    }
}

void AlarmTalker::_AlarmListener::catch_alarm(uint64_t alarmId, const std::unique_ptr<AlarmDefinition::AlarmMessage> &alarmMessage)
{
    auto shared = master.lock();
    if (shared)
    {
        nlohmann::json jAlarmMessage;
        jAlarmMessage["Id"] = alarmId;
        jAlarmMessage["Device"] = alarmMessage->deviceId;
        jAlarmMessage["Point"] = alarmMessage->pointId;
        jAlarmMessage["State"] = alarmMessage->alarmState;
        jAlarmMessage["ActiveValue"] = alarmMessage->activeValue.get_float();
        jAlarmMessage["ReferenceValue"] = alarmMessage->refValue.get_float();
        jAlarmMessage["Comparison"] = alarmMessage->comparison;
        jAlarmMessage["Time_ms"] = alarmMessage->msTime;
        shared->send_alarm(jAlarmMessage.dump());
    }
}
