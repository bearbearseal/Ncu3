#include "AlarmStorage.h"

using namespace std;

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

AlarmStorage::AlarmStorage(const std::string &dbFile) : theDb(dbFile)
{
    threadSocket = move(itc.create_fixed_socket(1, 2));
    senderSocket = move(itc.create_fixed_socket(2, 1));
    theProcess = nullptr;
}

AlarmStorage::~AlarmStorage()
{
    stop();
}

void AlarmStorage::start()
{
    if (theProcess == nullptr)
    {
        theProcess = make_unique<thread>(the_process, this);
    }
}

void AlarmStorage::stop()
{
    senderSocket->send_message(variant<Message2Thread, Message2Sender>(Message2Thread(Command::Stop, 0, nullptr)));
    theProcess->join();
    theProcess.reset();
}

vector<pair<uint64_t, AlarmStorage::AlarmMessage>> AlarmStorage::load_active_alarms()
{
    senderSocket->send_message(variant<Message2Thread, Message2Sender>(Message2Thread(Command::GetActiveAlarm, 0, nullptr)));
}

vector<pair<uint64_t, AlarmStorage::AlarmMessage>> AlarmStorage::load_history_alarms_begin_time(uint64_t msTime, size_t count)
{

}

vector<pair<uint64_t, AlarmStorage::AlarmMessage>> AlarmStorage::load_history_alarms_interval(uint64_t beginTime, uint64_t endTime)
{

}

void AlarmStorage::store_alarm(unique_ptr<AlarmMessage> &message)
{
    senderSocket->send_message(variant<Message2Thread, Message2Sender>(Message2Thread(Command::StoreAlarm, 0, move(message))));
}

uint64_t AlarmStorage::store_to_active_alarm(unique_ptr<AlarmMessage> &message)
{
    try
    {
        return theDb.execute_insert("Insert into ActiveAlarm (Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms)"
                                    "Values(%lu, %lu, %f, %f, %u, %u, %llu)",
                                    message->deviceId, message->pointId, message->activeValue, message->refValue,
                                    message->comparison, message->alarmState, message->msTime);
    }
    catch (Sqlite3::Exception e)
    {
        return 0;
    }
}

bool AlarmStorage::remove_from_active_alarm(uint64_t id)
{
    return theDb.execute_update("Delete from ActiveAlarm where Id=%llu", id);
}

void AlarmStorage::store_to_history_alarm(uint64_t id, unique_ptr<AlarmMessage> &message)
{
    theDb.execute_update("Insert into HistoryAlarm (Id, Device, Point, ActiveValue, RefValue, Comparison, AlarmState, Time_ms)"
                         "Values(%lu, %lu, %lu, %f, %f, %u, %u, %llu)",
                         id, message->deviceId, message->pointId, message->activeValue,
                         message->refValue, message->comparison, message->alarmState, message->msTime);
}

void AlarmStorage::handle_alarm(unique_ptr<AlarmMessage> &message)
{
    uint64_t meldId = meld_device_point_id(message->deviceId, message->pointId);
    //if previously not alarm
    if (!point2ActiveAlarmMap.count(meldId))
    {
        //if now is alarm
        if (message->activeValue != AlarmDefinition::AlarmState::NORMAL)
        {
            //from normal to alarm, create an alarm in active
            uint64_t alarmId = store_to_active_alarm(message);
            if (static_cast<bool>(alarmId))
            {
                point2ActiveAlarmMap.emplace(meldId, ActiveAlarmData(alarmId, move(message)));
            }
        }
    }
    else
    {
        ActiveAlarmData &theAlarm = point2ActiveAlarmMap[meldId];
        //if another alarm state
        if (message->alarmState == AlarmDefinition::AlarmState::NORMAL)
        {
            //From alarm to normal, create alarm in history table, delete from active table
            store_to_history_alarm(theAlarm.alarmId, message);
            remove_from_active_alarm(theAlarm.alarmId);
            point2ActiveAlarmMap.erase(meldId);
        }
        else if (theAlarm.message->alarmState != message->alarmState)
        {
            //from 1 state to another, move the alarm from active to history
            store_to_history_alarm(theAlarm.alarmId, message);
            remove_from_active_alarm(theAlarm.alarmId);
            //Create an alarm in active.
            uint64_t alarmId = store_to_active_alarm(message);
            if (static_cast<bool>(alarmId))
            {
                theAlarm.alarmId = alarmId;
                theAlarm.message = move(message);
            }
        }
    }
}

void AlarmStorage::the_process(AlarmStorage *me)
{
    while (1)
    {
        //wiat for message
        if (me->threadSocket->wait_message())
        {
            //process message
            do
            {
                auto message = me->threadSocket->get_message();
                switch (get<Message2Thread>(message.message).command)
                {
                case Command::Stop:
                    return;
                case Command::StoreAlarm:
                    me->handle_alarm(get<Message2Thread>(message.message).message);
                    break;
                case Command::GetActiveAlarm:

                    break;
                case Command::GetHistoryAlarm:
                    break;
                }
            } while (me->threadSocket->has_message());
        }
    }
}
