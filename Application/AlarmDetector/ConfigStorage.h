//Give info of alarm logic
//Give info of property alarm

#ifndef ConfigStorage_H_
#define ConfigStorage_H_
#include "../../../MyLib/Sqlite/Sqlite3.h"
#include "../../../MyLib/Basic/HashKey.h"
#include <unordered_map>

class ConfigStorage {
public:
    ConfigStorage(const std::string& dbFilePath);
    virtual ~ConfigStorage();

    struct NodeAlarmLogicData
    {
        std::string comparison;
        double value;
        uint32_t msWaitTime;
        float threshold;
        uint8_t category;
        std::string message;
        uint16_t code;
    };

    struct PropertyAlarmData
    {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
        uint64_t alarmId;
        uint16_t priority; 
        uint8_t category;
        std::string message;
        uint16_t code;
    };

    std::unordered_map<uint32_t, NodeAlarmLogicData> get_node_alarm_logic_data();
    std::vector<PropertyAlarmData> get_property_alarm_data();

private:
    Sqlite3 theDb;
};

#endif