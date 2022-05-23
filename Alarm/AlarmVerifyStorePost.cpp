#include "AlarmVerifyStorePost.h"

using namespace std;
using namespace AlarmDefinition;

uint64_t meld_device_point_id(uint32_t deviceId, uint32_t pointId)
{
    uint64_t retVal = deviceId;
    retVal <<= 32;
    retVal += pointId;
    return retVal;
}

pair<uint32_t, uint32_t> split_device_point_id(uint64_t meldId)
{
    return {static_cast<uint32_t>(meldId >> 32), static_cast<uint32_t>(meldId)};
}

AlarmVerifyStorePost::AlarmVerifyStorePost(const std::string &dbFile) : theDb(dbFile)
{
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms from ActiveAlarm");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint32_t deviceId = result->get_integer(i, "Device").second;
        uint32_t pointId = result->get_integer(i, "Point").second;
        uint64_t meldId = meld_device_point_id(deviceId, pointId);
        unique_ptr<AlarmMessage> alarmMessage = make_unique<AlarmMessage>(deviceId, pointId,
                                                                          result->get_float(i, "ActiveValue").second,
                                                                          result->get_float(i, "RefValue").second,
                                                                          AlarmDefinition::Comparison(result->get_integer(i, "Comparison").second),
                                                                          AlarmDefinition::AlarmState(result->get_integer(i, "AlarmState").second),
                                                                          result->get_integer(i, "Time_ms").second);
        uint64_t alarmId = result->get_integer(i, "Id").second;
        point2ActiveAlarmMap.emplace(meldId, ActiveAlarmData{alarmId, move(alarmMessage)});
    }
    serverSocket = itc.get_server_socket();
    //threadSocket = move(itc.create_fixed_socket(1, 2));
    //senderSocket = move(itc.create_fixed_socket(2, 1));
    theProcess = make_unique<thread>(the_process, this);
}

AlarmVerifyStorePost::~AlarmVerifyStorePost()
{
    stop();
}

void AlarmVerifyStorePost::catch_alarm_state(uint32_t equipmentId, uint32_t pointId, const Value &theValue, AlarmDefinition::AlarmState state,
                                             const Value &refValue, AlarmDefinition::Comparison compare, const chrono::time_point<chrono::system_clock> theMoment)
{
    auto msNow = theMoment.time_since_epoch();
    uint64_t msTime = chrono::duration_cast<chrono::milliseconds>(msNow).count();
    unique_ptr<AlarmMessage> alarmMessage = make_unique<AlarmMessage>(equipmentId, pointId, theValue, refValue, compare, state, msTime);
    store_alarm(alarmMessage);
}

void AlarmVerifyStorePost::start()
{
    if (theProcess == nullptr)
    {
        theProcess = make_unique<thread>(the_process, this);
    }
}

void AlarmVerifyStorePost::stop()
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server{Command::Stop});
    theProcess->join();
    theProcess.reset();
}

void AlarmVerifyStorePost::add_alarm_listener(std::weak_ptr<AlarmListener> listener)
{
    auto shared = listener.lock();
    if (shared != nullptr)
    {
        listenerMap[shared.get()] = listener;
    }
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::read_active_alarm()
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server{Command::GetActiveAlarm});
    if (clientSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
    {
        return clientSocket->get_message().value().alarms;
    }
    return ReadAlarmData();
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::read_active_alarm(uint64_t id)
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server(Command::GetActiveAlarmId, id));
    if (clientSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
    {
        return clientSocket->get_message().value().alarms;
    }
    return ReadAlarmData();
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::read_history_alarm_id(uint64_t id, size_t count)
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server(Command::GetActiveAlarm, id, count));
    if (clientSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
    {
        return clientSocket->get_message().value().alarms;
    }
    return ReadAlarmData();
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::read_history_alarm_count(uint64_t beginTime, size_t count)
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server(Command::GetActiveAlarm, beginTime, count));
    if (clientSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
    {
        return clientSocket->get_message().value().alarms;
    }
    return ReadAlarmData();
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::read_history_alarm_interval(uint64_t beginTime, uint64_t endTime)
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server(Command::GetActiveAlarm, beginTime, endTime));
    if (clientSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
    {
        return clientSocket->get_message().value().alarms;
    }
    return ReadAlarmData();
}

void AlarmVerifyStorePost::store_alarm(unique_ptr<AlarmMessage> &message)
{
    auto clientSocket = itc.get_client_socket();
    clientSocket->send_message(Message2Server(Command::StoreAlarm, message));
}

optional<uint64_t> AlarmVerifyStorePost::store_to_active_alarm(unique_ptr<AlarmMessage> &message)
{
    return theDb.execute_insert("Insert into ActiveAlarm (Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms)"
                                "Values(%lu, %u, %f, %f, %d, %d, %lu)",
                                message->deviceId, message->pointId, message->activeValue.get_float(), message->refValue.get_float(),
                                int(message->comparison), message->alarmState, message->msTime);
}

bool AlarmVerifyStorePost::remove_from_active_alarm(uint64_t id)
{
    return theDb.execute_update("Delete from ActiveAlarm where Id=%llu", id);
}

void AlarmVerifyStorePost::store_to_history_alarm(uint64_t id, unique_ptr<AlarmMessage> &message)
{
    theDb.execute_update("Insert into HistoryAlarm (Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms)"
                         "Values(%lu, %u, %u, %f, %f, %u, %d, %lu)",
                         id, message->deviceId, message->pointId, message->activeValue.get_float(),
                         message->refValue.get_float(), uint8_t(message->comparison), message->alarmState, message->msTime);
}

void AlarmVerifyStorePost::handle_alarm(unique_ptr<AlarmMessage> &message)
{
    uint64_t meldId = meld_device_point_id(message->deviceId, message->pointId);
    // if previously not alarm
    if (!point2ActiveAlarmMap.count(meldId))
    {
        // if now is alarm
        if (message->activeValue != AlarmDefinition::AlarmState::NORMAL)
        {
            printf("From normal to alarm.\n");
            // from normal to alarm, create an alarm in active
            optional<uint64_t> alarmId = store_to_active_alarm(message);
            if (alarmId.has_value())
            {
                point2ActiveAlarmMap.emplace(meldId, ActiveAlarmData{alarmId.value(), move(message)});
            }
        }
    }
    else
    {
        ActiveAlarmData &theAlarm = point2ActiveAlarmMap[meldId];
        // if another alarm state
        if (message->alarmState == AlarmDefinition::AlarmState::NORMAL)
        {
            printf("From alarm to normal.\n");
            // From alarm to normal, create alarm in history table, delete from active table
            store_to_history_alarm(theAlarm.alarmId, message);
            remove_from_active_alarm(theAlarm.alarmId);
            point2ActiveAlarmMap.erase(meldId);
        }
        else if (theAlarm.message->alarmState != message->alarmState)
        {
            printf("Change of alarm state.\n");
            // from 1 state to another, move the alarm from active to history
            store_to_history_alarm(theAlarm.alarmId, message);
            remove_from_active_alarm(theAlarm.alarmId);
            // Create an alarm in active.
            optional<uint64_t> alarmId = store_to_active_alarm(message);
            if (alarmId.has_value())
            {
                theAlarm.alarmId = alarmId.value();
                theAlarm.message = move(message);
            }
        }
    }
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::load_active_alarm()
{
    ReadAlarmData retVal;
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms"
                                      " From ActiveAlarm Limit 101");
    if (result->get_row_count() > 100)
    {
        retVal.alarmList.resize(100);
        retVal.continuation = result->get_integer(101, "Id").second;
    }
    else
    {
        retVal.alarmList.resize(result->get_row_count());
    }
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        pair<uint64_t, AlarmMessage> &entry = retVal.alarmList[i];
        entry.first = result->get_integer(i, "Id").second;
        entry.second.deviceId = result->get_integer(i, "Device").second;
        entry.second.pointId = result->get_integer(i, "Point").second;
        entry.second.activeValue = result->get_float(i, "ActiveValue").second;
        entry.second.refValue = result->get_float(i, "RefValue").second;
        entry.second.comparison = static_cast<AlarmDefinition::Comparison>(result->get_integer(i, "Comparison").second);
        entry.second.alarmState = static_cast<AlarmDefinition::AlarmState>(result->get_integer(i, "AlarmState").second);
        entry.second.msTime = result->get_integer(i, "Alarm_ms").second;
    }
    return retVal;
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::load_active_alarm(uint64_t id)
{
    ReadAlarmData retVal;
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms"
                                      " From ActiveAlarm Limit 101 Where Id>=%llu",
                                      id);
    if (result->get_row_count() > 100)
    {
        retVal.alarmList.resize(100);
        retVal.continuation = result->get_integer(101, "Id").second;
    }
    else
    {
        retVal.alarmList.resize(result->get_row_count());
    }
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        pair<uint64_t, AlarmMessage> &entry = retVal.alarmList[i];
        entry.first = result->get_integer(i, "Id").second;
        entry.second.deviceId = result->get_integer(i, "Device").second;
        entry.second.pointId = result->get_integer(i, "Point").second;
        entry.second.activeValue = result->get_float(i, "ActiveValue").second;
        entry.second.refValue = result->get_float(i, "RefValue").second;
        entry.second.comparison = static_cast<AlarmDefinition::Comparison>(result->get_integer(i, "Comparison").second);
        entry.second.alarmState = static_cast<AlarmDefinition::AlarmState>(result->get_integer(i, "AlarmState").second);
        entry.second.msTime = result->get_integer(i, "Alarm_ms").second;
    }
    return retVal;
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::load_history_alarm_id(uint64_t id, size_t count)
{
    if (count > 100)
    {
        count = 101;
    }
    else
    {
        ++count;
    }
    ReadAlarmData retVal;
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms"
                                      " From HistoryAlarm Where Id>=%llu Limit %zu",
                                      id, count);
    retVal.alarmList.resize(result->get_row_count());
    if (result->get_row_count() >= count)
    {
        retVal.continuation = result->get_integer(count, "Id").second;
    }
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        pair<uint64_t, AlarmMessage> &entry = retVal.alarmList[i];
        entry.first = result->get_integer(i, "Id").second;
        entry.second.deviceId = result->get_integer(i, "Device").second;
        entry.second.pointId = result->get_integer(i, "Point").second;
        entry.second.activeValue = result->get_float(i, "ActiveValue").second;
        entry.second.refValue = result->get_float(i, "RefValue").second;
        entry.second.comparison = static_cast<AlarmDefinition::Comparison>(result->get_integer(i, "Comparison").second);
        entry.second.alarmState = static_cast<AlarmDefinition::AlarmState>(result->get_integer(i, "AlarmState").second);
        entry.second.msTime = result->get_integer(i, "Alarm_ms").second;
    }
    return retVal;
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::load_history_alarm_count(uint64_t beginTime, size_t count)
{
    if (count > 100)
    {
        count = 101;
    }
    else
    {
        ++count;
    }
    ReadAlarmData retVal;
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms"
                                      " From HistoryAlarm Where Time_ms>=%llu Limit %zu",
                                      beginTime, count);
    retVal.alarmList.resize(result->get_row_count());
    if (result->get_row_count() >= count)
    {
        retVal.continuation = result->get_integer(count, "Id").second;
    }
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        pair<uint64_t, AlarmMessage> &entry = retVal.alarmList[i];
        entry.first = result->get_integer(i, "Id").second;
        entry.second.deviceId = result->get_integer(i, "Device").second;
        entry.second.pointId = result->get_integer(i, "Point").second;
        entry.second.activeValue = result->get_float(i, "ActiveValue").second;
        entry.second.refValue = result->get_float(i, "RefValue").second;
        entry.second.comparison = static_cast<AlarmDefinition::Comparison>(result->get_integer(i, "Comparison").second);
        entry.second.alarmState = static_cast<AlarmDefinition::AlarmState>(result->get_integer(i, "AlarmState").second);
        entry.second.msTime = result->get_integer(i, "Alarm_ms").second;
    }
    return retVal;
}

AlarmVerifyStorePost::ReadAlarmData AlarmVerifyStorePost::load_history_alarm_interval(uint64_t beginTime, uint64_t endTime)
{
    ReadAlarmData retVal;
    auto result = theDb.execute_query("Select Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms"
                                      " From HistoryAlarm Where Time_ms>=%llu AND Time_ms<=%llu Limit 101",
                                      beginTime, endTime);
    if (result->get_row_count() > 100)
    {
        retVal.alarmList.resize(100);
        retVal.continuation = result->get_integer(101, "Id").second;
    }
    else
    {
        retVal.alarmList.resize(result->get_row_count());
    }
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        pair<uint64_t, AlarmMessage> &entry = retVal.alarmList[i];
        entry.first = result->get_integer(i, "Id").second;
        entry.second.deviceId = result->get_integer(i, "Device").second;
        entry.second.pointId = result->get_integer(i, "Point").second;
        entry.second.activeValue = result->get_float(i, "ActiveValue").second;
        entry.second.refValue = result->get_float(i, "RefValue").second;
        entry.second.comparison = static_cast<AlarmDefinition::Comparison>(result->get_integer(i, "Comparison").second);
        entry.second.alarmState = static_cast<AlarmDefinition::AlarmState>(result->get_integer(i, "AlarmState").second);
        entry.second.msTime = result->get_integer(i, "Alarm_ms").second;
    }
    return retVal;
}

void AlarmVerifyStorePost::the_process(AlarmVerifyStorePost *me)
{
    while (1)
    {
        // wiat for message
        if (me->serverSocket->wait_message() == ServerClientItc<Message2Server, Message2Client>::WaitResult::GotMessage)
        {
            // process message
            do
            {
                auto messagePair = me->serverSocket->get_message();
                Message2Server &aMessage = messagePair.value().message;
                auto clientId = messagePair.value().sourceId;
                switch (aMessage.command)
                {
                case Command::Stop:
                    return;
                case Command::StoreAlarm:
                {
                    me->handle_alarm(aMessage.alarmMessage);
                    break;
                }
                case Command::GetActiveAlarm:
                {
                    auto result = me->load_active_alarm();
                    me->serverSocket->send_message(clientId, Message2Client{Command::GetActiveAlarm, result});
                    break;
                }
                case Command::GetActiveAlarmId:
                {
                    auto result = me->load_active_alarm(aMessage.parameter1);
                    me->serverSocket->send_message(clientId, Message2Client{Command::GetActiveAlarmId, result});
                }
                case Command::GetHistoryAlarmId:
                {
                    auto result = me->load_history_alarm_id(aMessage.parameter1, aMessage.parameter2);
                    me->serverSocket->send_message(clientId, Message2Client{Command::GetHistoryAlarmId, result});
                    break;
                }
                case Command::GetHistoryAlarmCount:
                {
                    auto result = me->load_history_alarm_count(aMessage.parameter1, aMessage.parameter2);
                    me->serverSocket->send_message(clientId, Message2Client{Command::GetHistoryAlarmId, result});
                    break;
                }
                case Command::GetHistoryAlarmInterval:
                {
                    auto result = me->load_history_alarm_interval(aMessage.parameter1, aMessage.parameter2);
                    me->serverSocket->send_message(clientId, Message2Client{Command::GetHistoryAlarmId, result});
                    break;
                }
                }
            } while (me->serverSocket->has_message());
        }
        else
        {
            // The itc got destructed, nothing logical could be done
            break;
        }
    }
}
