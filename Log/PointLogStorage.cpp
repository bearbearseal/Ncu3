#include "PointLogStorage.h"

using namespace std;

void PointLogStorage::TimerListener::catch_time_event(time_t eventTime, uint32_t token)
{
    master.catch_time_event(eventTime);
}

PointLogStorage::PointLogStorage(const std::string &sqliteName) : sqlite(sqliteName)
{
    EventTimer::start();
    // tempRecords.reserve(MAX_RECORDS_IN_RAM);
    timerListener = make_shared<TimerListener>(*this);
    threadSocket = theItc.create_socket(1);
    outsiderSocket = theItc.create_fixed_socket(2, 1);
    // Load the content of LogTable to RAM so the following load_record would be fast
    get_log_table_data();
    // Check the number of records in the current table.
    get_active_table_data();
    bulkInsert = sqlite.create_bulk_insert<int64_t, int64_t, int64_t, int64_t, string>("Insert Into " + activeTableData.name + " (Id, Device, Point, SecTime, Value)VALUES(?, ?, ?, ?, ?)");
    if (bulkInsert == nullptr)
    {
        printf("Failed to create bulk insert: %s.\n", string("Insert Into " + activeTableData.name + " (Id, Device, Point, SecTime, Value)VALUES(?, ?, ?, ?, ?)").c_str());
    }
    else
    {
        // Create an add empty timed event to run once every 1 minute
        EventTimer::add_time_event(chrono::time_point_cast<chrono::seconds>(chrono::system_clock::now()).time_since_epoch().count() + 60, timerListener, 1, PUSH_PERIOD);
    }
    // Start thread
    theThread = make_unique<thread>(thread_process, this);
}

PointLogStorage::~PointLogStorage()
{
    outsiderSocket->send_message(ThreadMessage{MessageCommand::Terminate, {}});
}

void PointLogStorage::store_record(const RecordData &record)
{
    outsiderSocket->send_message(ThreadMessage{MessageCommand::StoreRecord, vector<RecordData>{record}});
}

void PointLogStorage::store_record(vector<RecordData> &&records)
{
    outsiderSocket->send_message(ThreadMessage{MessageCommand::StoreRecord, StoreRecordData(records)});
}

PointLogStorage::LoadRecordReply PointLogStorage::load_record_by_interval(time_t beginTime, time_t endTime)
{
    LoadRecordReply retVal;
    unique_ptr<ITC<ThreadMessage>::FixedSocket> socket;
    {
        lock_guard<mutex> lock(socketIdMutex);
        socket = theItc.create_fixed_socket(nextSocketId, 1);
        ++nextSocketId; // Let it grow, no need extra steps to move it back to 3 since the number can grow very high
    }
    ThreadMessage message{MessageCommand::LoadRecordByInterval, LoadRecordByIntervalQuery{beginTime, endTime}};
    socket->send_message(message);
    if (socket->wait_message())
    {
        auto reply = socket->get_message();
        ThreadMessage &replyMessage = reply.message;
        LoadRecordReply &loadReply = get<LoadRecordReply>(replyMessage.data);
        retVal.records = move(loadReply.records);
        retVal.continuation = loadReply.continuation;
        retVal.hasMore = loadReply.hasMore;
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::load_record_by_count(time_t beginTime, size_t count)
{
    LoadRecordReply retVal;
    unique_ptr<ITC<ThreadMessage>::FixedSocket> socket;
    {
        lock_guard<mutex> lock(socketIdMutex);
        socket = theItc.create_fixed_socket(nextSocketId, 1);
        ++nextSocketId; // Let it grow, no need extra steps to move it back to 3 since the number can grow very high
    }
    ThreadMessage message{MessageCommand::LoadRecordByCount, LoadRecordByCountQuery{beginTime, count}};
    socket->send_message(message);
    if (socket->wait_message())
    {
        auto reply = socket->get_message();
        ThreadMessage &replyMessage = reply.message;
        LoadRecordReply &loadReply = get<LoadRecordReply>(replyMessage.data);
        retVal.records = move(loadReply.records);
        retVal.continuation = loadReply.continuation;
        retVal.hasMore = loadReply.hasMore;
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::load_record_by_interval_continue(const std::pair<time_t, uint64_t> &continuation, time_t endTime)
{
    LoadRecordReply retVal;
    unique_ptr<ITC<ThreadMessage>::FixedSocket> socket;
    {
        lock_guard<mutex> lock(socketIdMutex);
        socket = theItc.create_fixed_socket(nextSocketId, 1);
        ++nextSocketId;
    }
    ThreadMessage message{MessageCommand::LoadRecordByIntervalContinue, LoadRecordByIntervalContinueQuery{continuation, endTime}};
    socket->send_message(message);
    if (socket->wait_message())
    {
        auto reply = socket->get_message();
        ThreadMessage &replyMessage = reply.message;
        LoadRecordReply &loadReply = get<LoadRecordReply>(replyMessage.data);
        retVal.records = move(loadReply.records);
        retVal.continuation = loadReply.continuation;
        retVal.hasMore = loadReply.hasMore;
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::load_record_by_count_continue(const std::pair<time_t, uint64_t> &continuation, size_t count)
{
    LoadRecordReply retVal;
    unique_ptr<ITC<ThreadMessage>::FixedSocket> socket;
    {
        lock_guard<mutex> lock(socketIdMutex);
        socket = theItc.create_fixed_socket(nextSocketId, 1);
        ++nextSocketId;
    }
    ThreadMessage message{MessageCommand::LoadRecordByCountContinue, LoadRecordByCountContinueQuery{continuation, count}};
    socket->send_message(message);
    if (socket->wait_message())
    {
        auto reply = socket->get_message();
        ThreadMessage &replyMessage = reply.message;
        LoadRecordReply &loadReply = get<LoadRecordReply>(replyMessage.data);
        retVal.records = move(loadReply.records);
        retVal.continuation = loadReply.continuation;
        retVal.hasMore = loadReply.hasMore;
    }
    return retVal;
}

bool PointLogStorage::create_and_use_new_log_table(time_t firstRecordTime)
{
    activeTableData.name = "table" + to_string(startTime2TableMap.size() + 1);
    activeTableData.count = 0;
    return sqlite.execute_atomic_update({"Create Table \"" + activeTableData.name +
                                             "\" (\"Id\" INTEGER PRIMARY Key,"
                                             "\"Device\" INTEGER NOT NULL,"
                                             "\"Point\" INTEGER NOT NULL,"
                                             "\"SecTime\" INTEGER NOT NULL,"
                                             "\"Value\" TEXT NOT NULL)",
                                         "Insert Into TableInfo (Name, BeginSec) VALUES (\"" +
                                             activeTableData.name + "\", " + to_string(firstRecordTime) + ")"});
}

void PointLogStorage::get_log_table_data()
{
    auto resultSet = sqlite.execute_query("Select Name, BeginSec from TableInfo");
    for (size_t i = 0; i < resultSet->get_row_count(); ++i)
    {
        time_t startTime = resultSet->get_integer(i, "BeginSec").second;
        startTime2TableMap[startTime] = resultSet->get_string(i, "Name").second;
    }
    if (startTime2TableMap.empty())
    {
        create_and_use_new_log_table(std::chrono::system_clock::to_time_t(chrono::system_clock::now()));
    }
}

void PointLogStorage::get_active_table_data()
{
    auto lastTable = startTime2TableMap.rbegin();
    if (lastTable != startTime2TableMap.rend())
    {
        activeTableData.name = lastTable->second;
        auto result = sqlite.execute_query("Select Max(Id) from " + activeTableData.name);
        activeTableData.count = result->get_integer(0, "Max(Id)").second;
    }
}

void PointLogStorage::catch_time_event(time_t theMoment)
{
    // ThreadMessage message{MessageCommand::PeriodicPush, 1};
    outsiderSocket->send_message(ThreadMessage{MessageCommand::PeriodicPush, {}});
}

void PointLogStorage::perform_push_to_sqlite()
{
    printf("Periodic pushing.\n");
    auto result = bulkInsert->commit_insert();
    if (!result.has_value())
    {
        printf("Push error.\n");
    }
}

void PointLogStorage::perform_store_log(const std::vector<RecordData> &records)
{
    size_t totalRecords = bulkInsert->count_line() + records.size();
    // Check if the size is over 1000.
    if (totalRecords >= MAX_RECORDS_IN_RAM)
    {
        if (activeTableData.count + totalRecords >= MAX_RECORDS_PER_TABLE)
        {
            bulkInsert->commit_insert();
            create_and_use_new_log_table(records.begin()->secTime);
        }
        else
        {
            for (size_t i = 0; i < records.size(); ++i)
            {
                bulkInsert->add_line(activeTableData.count + i + 1, records[i].device, records[i].point, records[i].secTime, records[i].value.to_string());
                ++activeTableData.count;
            }
            bulkInsert->commit_insert();
        }
    }
    else
    {
        for (size_t i = 0; i < records.size(); ++i)
        {
            bulkInsert->add_line(activeTableData.count + i + 1, records[i].device, records[i].point, records[i].secTime, records[i].value.to_string());
            ++activeTableData.count;
        }
    }
}

PointLogStorage::LoadRecordReply PointLogStorage::perform_load_log_by_interval(time_t beginTime, time_t endTime)
{
    LoadRecordReply retVal;
    // Check which table are covered
    auto beginTable = startTime2TableMap.lower_bound(beginTime); // Iterator to 1st entry that is >= beginTime
    auto endTable = startTime2TableMap.lower_bound(endTime);     // Iterator to 1st entry that is >= endTime
    if (beginTable != startTime2TableMap.end())
    {
        // beginBound  exist
        if (beginTable->first != beginTime && beginTable != startTime2TableMap.begin())
        {
            // beginTable time now has time after beginTime and there exist a table with earlier time
            --beginTable;
        }
    }
    else
    {
        // no table that has time >= beginTime
        return retVal;
    }
    for (auto i = beginTable; i != endTable && retVal.records.size() < MAX_LOAD_COUNT; ++i)
    {
        // load from beginTable, load 1 extra to know whether extra exist.
        size_t total = MAX_LOAD_COUNT - retVal.records.size() + 1;
        auto result = sqlite.execute_query("Select Id, Device, Point, SecTime, Value from %s where SecTime >= %u AND SecTime <= %u Limit %zu\n",
                                           i->second.c_str(), beginTime, endTime, total);
        for (size_t j = 0; j < result->get_row_count(); ++j)
        {
            retVal.records.push_back(RecordData{
                uint16_t(result->get_integer(j, "Device").second),
                uint16_t(result->get_integer(j, "Point").second),
                result->get_integer(j, "SecTime").second,
                result->get_float(j, "Value").second});
        }
        if (result->get_column_count() == total)
        {
            // Got more
            retVal.continuation.first = i->first;
            retVal.continuation.second = result->get_integer(result->get_row_count() - 1, "Id").second;
            retVal.hasMore = true;
        }
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::perform_load_log_by_count(time_t beginTime, size_t count)
{
    LoadRecordReply retVal;
    auto beginTable = startTime2TableMap.lower_bound(beginTime); // Iterator to 1st entry that is >= beginTime
    if (beginTable != startTime2TableMap.end())
    {
        // beginBound  exist
        if (beginTable->first != beginTime && beginTable != startTime2TableMap.begin())
        {
            // beginTable time now has time after beginTime and there exist a table with earlier time
            --beginTable;
        }
    }
    else
    {
        // no table that has time >= beginTime
        return retVal;
    }
    if (count > MAX_LOAD_COUNT)
    {
        count = MAX_LOAD_COUNT;
    }
    for (auto i = beginTable; i != startTime2TableMap.end() && retVal.records.size() < MAX_LOAD_COUNT; ++i)
    {
        // load from beginTable, load 1 extra to know whether extra exist.
        size_t total = MAX_LOAD_COUNT - retVal.records.size() + 1;
        auto result = sqlite.execute_query("Select Id, Device, Point, SecTime, Value from %s where SecTime >= %u Limit %zu\n",
                                           i->second.c_str(), beginTime, total);
        for (size_t j = 0; j < result->get_row_count(); ++j)
        {
            retVal.records.push_back(RecordData{
                uint16_t(result->get_integer(j, "Device").second),
                uint16_t(result->get_integer(j, "Point").second),
                result->get_integer(j, "SecTime").second,
                result->get_float(j, "Value").second});
        }
        if (result->get_column_count() == total)
        {
            // Got more
            retVal.continuation.first = i->first;
            retVal.continuation.second = result->get_integer(result->get_row_count() - 1, "Id").second;
            retVal.hasMore = true;
        }
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::perform_load_log_by_interval_continue(const pair<time_t, uint64_t> &continuation, time_t endTime)
{
    LoadRecordReply retVal;
    auto beginTable = startTime2TableMap.find(continuation.first);
    if (beginTable == startTime2TableMap.end())
    {
        return retVal;
    }
    auto endTable = startTime2TableMap.lower_bound(endTime);
    for (auto i = beginTable; i != endTable && retVal.records.size() < MAX_LOAD_COUNT; ++i)
    {
        // load from beginTable, load 1 extra to know whether extra exist.
        size_t total = MAX_LOAD_COUNT - retVal.records.size() + 1;
        auto result = sqlite.execute_query("Select Id, Device, Point, SecTime, Value from %s where Id >= %u AND SecTime <= %u Limit %zu\n",
                                           i->second.c_str(), continuation.second, endTime, total);
        for (size_t j = 0; j < result->get_row_count(); ++j)
        {
            retVal.records.push_back(RecordData{
                uint16_t(result->get_integer(j, "Device").second),
                uint16_t(result->get_integer(j, "Point").second),
                result->get_integer(j, "SecTime").second,
                result->get_float(j, "Value").second});
        }
        if (result->get_column_count() == total)
        {
            // Got more
            retVal.continuation.first = i->first;
            retVal.continuation.second = result->get_integer(result->get_row_count() - 1, "Id").second;
            retVal.hasMore = true;
        }
    }
    return retVal;
}

PointLogStorage::LoadRecordReply PointLogStorage::perform_load_log_by_count_continue(const std::pair<time_t, uint64_t> &continuation, size_t count)
{
    LoadRecordReply retVal;
    auto beginTable = startTime2TableMap.find(continuation.first);
    if (beginTable == startTime2TableMap.end())
    {
        return retVal;
    }
    for (auto i = beginTable; i != startTime2TableMap.end() && retVal.records.size() < MAX_LOAD_COUNT; ++i)
    {
        // load from beginTable, load 1 extra to know whether extra exist.
        size_t total = MAX_LOAD_COUNT - retVal.records.size() + 1;
        auto result = sqlite.execute_query("Select Id, Device, Point, SecTime, Value from %s where Id >= %u Limit %zu\n",
                                           i->second.c_str(), continuation.second, total);
        for (size_t j = 0; j < result->get_row_count(); ++j)
        {
            retVal.records.push_back(RecordData{
                uint16_t(result->get_integer(j, "Device").second),
                uint16_t(result->get_integer(j, "Point").second),
                result->get_integer(j, "SecTime").second,
                result->get_float(j, "Value").second});
        }
        if (result->get_column_count() == total)
        {
            // Got more
            retVal.continuation.first = i->first;
            retVal.continuation.second = result->get_integer(result->get_row_count() - 1, "Id").second;
            retVal.hasMore = true;
        }
    }
    return retVal;
}

void PointLogStorage::thread_process(PointLogStorage *me)
{
    while (me->threadSocket->wait_message())
    {
        do
        {
            auto message = me->threadSocket->get_message();
            switch (message.message.command)
            {
            case MessageCommand::Terminate:
                printf("Log shutting down.\n");
                return;
            case MessageCommand::PeriodicPush:
                me->perform_push_to_sqlite();
                break;
            case MessageCommand::StoreRecord:
                me->perform_store_log(get<StoreRecordData>(message.message.data));
                break;
            case MessageCommand::LoadRecordByInterval:
            {
                LoadRecordByIntervalQuery &parameters = get<LoadRecordByIntervalQuery>(message.message.data);
                me->threadSocket->message_to_socket(
                    message.sourceSocketId,
                    ThreadMessage{MessageCommand::LoadRecordByInterval, me->perform_load_log_by_interval(parameters.beginTime, parameters.endTime)});
                break;
            }
            case MessageCommand::LoadRecordByCount:
            {
                LoadRecordByCountQuery &parameters = get<LoadRecordByCountQuery>(message.message.data);
                me->threadSocket->message_to_socket(
                    message.sourceSocketId,
                    ThreadMessage{MessageCommand::LoadRecordByCount, me->perform_load_log_by_count(parameters.beginTime, parameters.count)});
                break;
            }
            case MessageCommand::LoadRecordByIntervalContinue:
            {
                LoadRecordByIntervalContinueQuery &parameters = get<LoadRecordByIntervalContinueQuery>(message.message.data);
                me->threadSocket->message_to_socket(
                    message.sourceSocketId,
                    ThreadMessage{MessageCommand::LoadRecordByIntervalContinue, me->perform_load_log_by_interval_continue(parameters.continuation, parameters.endTime)});
                break;
            }
            case MessageCommand::LoadRecordByCountContinue:
            {
                LoadRecordByCountContinueQuery &parameters = get<LoadRecordByCountContinueQuery>(message.message.data);
                me->threadSocket->message_to_socket(
                    message.sourceSocketId,
                    ThreadMessage{MessageCommand::LoadRecordByCountContinue, me->perform_load_log_by_count_continue(parameters.continuation, parameters.count)});
                break;
            }
            }
        } while (me->threadSocket->has_message());
    }
}
