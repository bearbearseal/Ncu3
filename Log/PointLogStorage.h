#ifndef _PointLogStorage_H_
#define _PointLogStorage_H_
#include <vector>
#include <map>
#include <variant>
#include "../../MyLib/ITC/ITC.h"
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/Basic/Value.h"
#include "../../MyLib/Timer/EventTimer.h"

/*********************************************************************************************************
 * Each time a table have more than 1,000,000 records, it would create a new table to store new log data.
 * New table to have name contains the time of the 1st record.
 * New records added to this object would stay in temporary memory.
 * The records stored to sqlite3 mechanism:
 * 1. Every minutes, what's in temporary memory would be checked.  If the oldest data is more than a minute old, everything would be stored.
 * 2. If total records in temporary memory is more than 1000 records.
 * 3. If total table is more than 100, the oldest is deleted.
 *********************************************************************************************************/
class PointLogStorage
{
    friend class TimerLister;

public:
    const size_t MAX_LOAD_COUNT = 1000;
    const uint16_t PUSH_PERIOD = 120;
    struct RecordData
    {
        uint16_t device;
        uint16_t point;
        time_t secTime;
        Value value;
    };
    struct LoadRecordReply
    {
        LoadRecordReply() {}
        LoadRecordReply(const LoadRecordReply &theOther)
        {
            records = theOther.records;
            continuation = theOther.continuation;
            hasMore = theOther.hasMore;
        }
        LoadRecordReply(LoadRecordReply &&theOther)
        {
            records = move(theOther.records);
            continuation = move(theOther.continuation);
            hasMore = theOther.hasMore;
        }
        LoadRecordReply &operator=(LoadRecordReply &&theOther)
        {
            records = move(theOther.records);
            continuation = move(theOther.continuation);
            hasMore = theOther.hasMore;
            return *this;
        }
        // implement operator here
        std::vector<RecordData> records;
        std::pair<time_t, uint64_t> continuation;
        bool hasMore = false;
    };
    PointLogStorage(const std::string &sqliteName);
    ~PointLogStorage();

    void store_record(const RecordData &record);
    void store_record(std::vector<RecordData> &&records);
    LoadRecordReply load_record_by_interval(time_t beginTime, time_t endTime = 0);
    LoadRecordReply load_record_by_count(time_t beginTime, size_t count = 0);
    LoadRecordReply load_record_by_interval_continue(const std::pair<time_t, uint64_t> &continuation, time_t endTime = 0);
    LoadRecordReply load_record_by_count_continue(const std::pair<time_t, uint64_t> &continuation, size_t count = 0);

private:
    struct RamRecordData
    {
        uint16_t device;
        uint16_t point;
        Value value;
    };
    const size_t MAX_RECORDS_IN_RAM = 1024;
    const size_t MAX_RECORDS_PER_TABLE = 1024 * 1024;
    bool create_and_use_new_log_table(time_t firstRecordTime);
    void get_log_table_data();
    void get_active_table_data();
    void catch_time_event(time_t theMoment);
    void perform_push_to_sqlite();
    void perform_store_log(const std::vector<RecordData> &records);
    LoadRecordReply perform_load_log_by_interval(time_t beginTime, time_t endTime = 0);
    LoadRecordReply perform_load_log_by_count(time_t beginTime, size_t count);
    LoadRecordReply perform_load_log_by_interval_continue(const std::pair<time_t, uint64_t> &continuation, time_t endTime = 0);
    LoadRecordReply perform_load_log_by_count_continue(const std::pair<time_t, uint64_t> &continuation, size_t count = 0);

    class TimerListener : public EventTimer::Listener
    {
    public:
        TimerListener(PointLogStorage &_master) : master(_master) {}
        ~TimerListener() {}
        void catch_time_event(time_t eventTime, uint32_t token);

    private:
        PointLogStorage &master;
    };
    enum class MessageCommand
    {
        Terminate,
        StoreRecord,
        PeriodicPush,
        LoadRecordByInterval,
        LoadRecordByCount,
        LoadRecordByIntervalContinue,
        LoadRecordByCountContinue
    };
    // Continuation point contains table name and record id, record id is the id of the last record.
    // Continuation point would exist even if no more data, the has more would tell if there are more data.
    struct LoadRecordByIntervalQuery
    {
        time_t beginTime;
        time_t endTime;
    };
    struct LoadRecordByCountQuery
    {
        time_t beginTime;
        size_t count;
    };
    struct LoadRecordByIntervalContinueQuery
    {
        const std::pair<time_t, uint64_t> &continuation; // time_t is the key to startTime2TableMap entry, uint64_t is the id of the line.
        time_t endTime;
    };
    struct LoadRecordByCountContinueQuery
    {
        const std::pair<time_t, uint64_t> &continuation;
        size_t count;
    };
    typedef std::vector<RecordData> StoreRecordData;
    struct ThreadMessage
    {
        MessageCommand command;
        std::variant<std::monostate,
                     LoadRecordByIntervalQuery,
                     LoadRecordByIntervalContinueQuery,
                     LoadRecordByCountQuery,
                     LoadRecordByCountContinueQuery,
                     LoadRecordReply,
                     StoreRecordData>
            data;
    };
    struct ActiveTableData
    {
        std::string name;
        size_t count;
    } activeTableData;

    // Let myTime got destroyed b4 timerListener
    // Timer myTimer;
    std::shared_ptr<TimerListener> timerListener;
    // Let bulkInsert destructs after sqlite
    Sqlite3 sqlite;
    std::chrono::system_clock::time_point bulkInsert1stRecordTime;
    // RecordId, Device, Point, TimeSec, Value
    std::unique_ptr<Sqlite3::BulkInsert<int64_t, int64_t, int64_t, int64_t, std::string>> bulkInsert;
    std::map<time_t, std::string> startTime2TableMap;
    ITC<ThreadMessage> theItc;
    std::mutex socketIdMutex;
    // The other socket took 1 and 2, other sockets can only be 3 onwards.
    size_t nextSocketId = 3;
    std::unique_ptr<ITC<ThreadMessage>::FixedSocket> outsiderSocket;
    std::unique_ptr<ITC<ThreadMessage>::Socket> threadSocket;
    std::unique_ptr<std::thread> theThread;
    static void thread_process(PointLogStorage *me);
};

#endif