#ifndef _PointLogKeeper_H_
#define _PointLogKeeper_H_
#include <cstdint>
#include <chrono>
#include "../../MyLib/Basic/Value.h"
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/ITC/ITC.h"

class PointLogKeeper {
public:
    struct LogData
    {
        uint64_t id;
        uint32_t device;
        uint32_t point;
        uint64_t millisecond;
        Value value;
    };
    PointLogKeeper(const std::string& filename);
    virtual ~PointLogKeeper();

    bool write_point_log(uint32_t device, uint32_t point, std::chrono::time_point<std::chrono::system_clock> theMoment, const Value& value);
    std::vector<LogData> read_point_log(uint64_t id);

private:
    enum class InternalCommand
    {
        AddLog,
        ReadLog,
        Terminate
    };
    struct InternalMessage
    {

    };
    Sqlite3 storage;
    
};

#endif