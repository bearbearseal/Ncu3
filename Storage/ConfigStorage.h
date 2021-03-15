#ifndef _ConfigStorage_H_
#define _ConfigStorage_H_
#include <string>
#include <vector>
#include <unordered_map>

#include "../../MyLib/Sqlite/Sqlite3.h"

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
        uint16_t pointId;
        uint16_t address;
        uint8_t type;
    };

    std::unordered_map<uint16_t, std::vector<ModbusIpPoint>> get_modbus_ip_point();

    struct ModbusRtuPoint {
        uint16_t pointId;
        uint16_t address;
        uint8_t type;
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

private:
    Sqlite3 theDb;
};

#endif