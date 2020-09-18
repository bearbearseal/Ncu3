#ifndef _AlarmStorage_H_
#define _AlarmStorage_H_
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "AlarmDefinition.h"
#include "../Basic/HashKey.h"
#include <vector>
#include <unordered_map>
#include <utility>
#include <list>
#include <mutex>

//This class cannot be entered from multi-thread
//No mutex coz worry this class would be destructed while mutex is locked
class AlarmStorage {
public:
    struct PropertyCondition {
        const HashKey::EitherKey equipment;
        const HashKey::EitherKey property;
        AlarmDefinition::Condition condition;
    };
    struct UnreportedAlarm {
        uint64_t id;
        uint64_t priorId;
        std::string equipment;
        std::string property;
        double value;
        uint64_t timeMilliSec;
        std::string message;
        size_t type;
        size_t code;
    };
    AlarmStorage(const std::string& dbName);
    ~AlarmStorage();
    //Returns the id
    size_t store_alarm(const AlarmDefinition::AlarmMessage& alarmMessage);
    std::vector<PropertyCondition> get_property_condition() const;
    std::list<UnreportedAlarm> get_unreported_alarms(size_t count) const;
    void remove_unreported_alarm(uint64_t id);
    void remove_unreported_alarms(std::vector<uint64_t> id);

private:
    struct ConditionPriorId {
        AlarmDefinition::Condition condition;
        uint64_t id;
    };
typedef std::unordered_map<HashKey::EitherKey, std::unordered_map<HashKey::EitherKey, ConditionPriorId, HashKey::EitherKey>, HashKey::EitherKey> ConditionMap;
    //std::mutex mapMutex;
    ConditionMap conditionMap;
    std::unique_ptr<Sqlite3> theDb;
};

#endif