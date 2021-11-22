#ifndef _ConfigStorage_H_
#define _ConfigStorage_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/Basic/Value.h"
#include "../../MyLib/Basic/HashKey.h"

class ConfigStorage {
public:
    ConfigStorage(const std::string& dbFile);
    virtual ~ConfigStorage();

    struct SerialPortData {
        uint32_t baudrate;
        bool paritybit;
        bool stopbit;
        bool hardwareFlowControl;
        bool softwareFlowControl;
        uint8_t bitPerByte;
        uint16_t msDelay;
    };

    std::unordered_map<std::string, SerialPortData> get_serial_port_data();

    struct ModbusIpChannelData {
        uint16_t deviceId;
        std::string hisAddress;
        uint16_t hisPort;
        std::string tagName;
        bool bigEndian;
        uint16_t maxRegister;
        uint16_t maxCoil;
        uint16_t msTimeout;
    };

    std::vector<ModbusIpChannelData> get_modbus_ip_channel_data();

    struct ModbusIpPoint {
        uint32_t pointId;
        uint32_t address;
        uint8_t type;
        uint32_t inOp;
        uint32_t outOp;
        //uint32_t alarmLogic;
    };

    std::unordered_map<uint16_t, std::vector<ModbusIpPoint>> get_modbus_ip_point();

    struct ModbusRtuPoint {
        uint16_t pointId;
        uint16_t address;
        uint8_t type;
        uint32_t inOp;
        uint32_t outOp;
        //uint32_t alarmLogic;
    };

    std::unordered_map<uint16_t, std::vector<ModbusRtuPoint>> get_modbus_rtu_point();

    struct ModbusRtuChannelData {
        uint16_t deviceId;
        std::string serialPortName;
        uint8_t slaveId;
        std::string tagName;
        bool bigEndian;
        uint16_t maxRegister;
        uint16_t maxCoil;
        uint16_t msTimeout;
    };

    std::vector<ModbusRtuChannelData> get_modbus_rtu_channel_data();

    struct EquipmentData {
        uint16_t equipmentId;
        std::string name;
    };

    std::vector<EquipmentData> get_equipment_data();

    struct PropertyData {
        std::string name;
        uint16_t deviceId;
        uint16_t pointId;
        uint16_t inOperation;
        uint16_t outOperation;
    };

    std::unordered_map<uint16_t, std::vector<PropertyData>> get_property_data();
/*
    struct ScheduleRuleConditionData {
        //size_t subject;
        std::string subject;
        //size_t comparison;
        std::string comparison;
        uint16_t value;
    };

    std::unordered_map<uint16_t, std::vector<ScheduleRuleConditionData>> get_schedule_condition_data();

    //Get conditions of each scheduleRule
    std::unordered_map<uint16_t, std::vector<uint16_t>> get_schedule_rule_and_condition_data();

    struct ScheduleData {
        uint16_t ruleId;
        uint8_t priority;
        uint16_t timeTableId;
    };
    std::unordered_map<uint16_t, std::vector<ScheduleData>> get_schedule_data();

    struct TimeTableData {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        std::string eventType;
        //size_t eventType;
        std::string valueString;
    };
    std::unordered_map<uint16_t, std::vector<TimeTableData>> get_time_table_data();

    struct EquipmentScheduleData 
    {
        uint16_t equipmentId;
        std::string propertyName;
        uint16_t scheduleId;
    };
    std::vector<EquipmentScheduleData> get_equipment_schedule_data();
*/
    struct AlarmLogicData {
        uint32_t compare;
        double refValue;
        uint32_t state;
    };
    struct AlarmLogicsData {
        uint32_t id;
        std::vector<AlarmLogicData> logicData;
    };
    std::vector<AlarmLogicsData> get_alarm_logic();

    struct NodeAlarmData {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
        uint32_t alarmLogicId;
        uint16_t priority;
    };
    std::vector<NodeAlarmData> get_node_alarm();

    struct NodeNormalMessage {
        HashKey::EitherKey equipmentId;
        HashKey::EitherKey propertyId;
        std::string message;
    };
    std::vector<NodeNormalMessage> get_normal_message();

    struct PointAlarmPair {
        uint32_t deviceId;
        uint32_t pointId;
        uint32_t logicGroupId;
    };
    std::vector<PointAlarmPair> get_alarm_point_pair();

    struct TimedAction
    {
        uint16_t action;
        double value;
        uint32_t time;  //hhmmss
    };
    struct TimeTableData
    {
        uint32_t id;
        std::vector<TimedAction> actionList; 
    };
    std::vector<TimeTableData> get_time_table();

    struct ScheduleRule
    {
        uint8_t subject;
        uint8_t compare;
        uint32_t value;
    };
    //GroupId2Rules, rules in sublist have and condition, or between list
    std::unordered_map<uint32_t, std::vector<std::vector<ScheduleRule>>> get_schedule_rule();

    struct RuleTablePair
    {
        uint32_t scheduleRule;
        uint32_t timeTable;
    };
    struct RuleTableData
    {
        uint32_t id;
        std::vector<RuleTablePair> pairList;
        uint32_t defaultTable;
    };
    std::vector<RuleTableData> get_schedule();

    struct PointSchedulePair
    {
        uint32_t deviceId;
        uint32_t pointId;
        uint32_t scheduleId;
    };
    std::vector<PointSchedulePair> get_point_schedule_pair();
    
private:
    Sqlite3 theDb;
};

#endif