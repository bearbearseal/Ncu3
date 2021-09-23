#include "ConfigStorage.h"

using namespace std;

ConfigStorage::ConfigStorage(const string &dbFile) : theDb(dbFile)
{
}

ConfigStorage::~ConfigStorage()
{
}

unordered_map<string, ConfigStorage::SerialPortData> ConfigStorage::get_serial_port_data()
{
    unordered_map<string, SerialPortData> retVal;
    auto result = theDb.execute_query("Select Name, Baudrate, ParityBit, StopBit, HardwareFlowControl, SoftwareFlowControl, BitPerByte, Delay_ms from SerialPortConfig");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        string name = result->get_string(i, "Name").second;
        SerialPortData &entry = retVal[name];
        entry.baudrate = result->get_integer(i, "Baudrate").second;
        entry.paritybit = result->get_integer(i, "ParityBit").second;
        entry.stopbit = result->get_integer(i, "StopBit").second;
        entry.hardwareFlowControl = result->get_integer(i, "HardwareFlowControl").second;
        entry.softwareFlowControl = result->get_integer(i, "SoftwareFlowControl").second;
        entry.bitPerByte = result->get_integer(i, "BitPerByte").second;
        entry.msDelay = result->get_integer(i, "Delay_ms").second;
    }
    return retVal;
}

vector<ConfigStorage::ModbusIpChannelData> ConfigStorage::get_modbus_ip_channel_data()
{
    vector<ModbusIpChannelData> retVal;
    auto result = theDb.execute_query("Select DeviceId, SlaveAddress, SlavePort, TagName, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusIpSlave");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        ModbusIpChannelData entry;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.hisAddress = result->get_string(i, "SlaveAddress").second;
        entry.hisPort = result->get_integer(i, "SlavePort").second;
        entry.tagName = result->get_string(i, "TagName").second;
        entry.bigEndian = result->get_integer(i, "BigEndian").second;
        entry.maxRegister = result->get_integer(i, "MaxRegister").second;
        entry.maxCoil = result->get_integer(i, "MaxCoil").second;
        entry.msTimeout = result->get_integer(i, "Timeout_ms").second;
        retVal.push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ModbusIpPoint>> ConfigStorage::get_modbus_ip_point()
{
    unordered_map<uint16_t, vector<ModbusIpPoint>> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type from ModbusIpPoint");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusIpPoint entry;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
        retVal[deviceId].push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ModbusRtuPoint>> ConfigStorage::get_modbus_rtu_point()
{
    unordered_map<uint16_t, vector<ModbusRtuPoint>> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type from ModbusRtuPoint");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusRtuPoint entry;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
        retVal[deviceId].push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::ModbusRtuChannelData> ConfigStorage::get_modbus_rtu_channel_data()
{
    vector<ModbusRtuChannelData> retVal;
    auto result = theDb.execute_query("Select DeviceId, SerialPortName, SlaveId, TagName, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusRtuSlave");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        ModbusRtuChannelData entry;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.serialPortName = result->get_string(i, "SerialPortName").second;
        entry.slaveId = result->get_integer(i, "SlaveId").second;
        entry.tagName = result->get_string(i, "TagName").second;
        entry.bigEndian = result->get_integer(i, "BigEndian").second;
        entry.maxRegister = result->get_integer(i, "MaxRegister").second;
        entry.maxCoil = result->get_integer(i, "MaxCoil").second;
        entry.msTimeout = result->get_integer(i, "Timeout_ms").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::EquipmentData> ConfigStorage::get_equipment_data()
{
    vector<EquipmentData> retVal;
    auto result = theDb.execute_query("Select Id, Name from Equipment");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        EquipmentData entry;
        entry.equipmentId = result->get_integer(i, "Id").second;
        entry.name = result->get_string(i, "Name").second;
        retVal.push_back(move(entry));
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::PropertyData>> ConfigStorage::get_property_data()
{
    unordered_map<uint16_t, vector<PropertyData>> retVal;
    auto result = theDb.execute_query("Select EquipmentId, Name, DeviceId, PointId, InOp, OutOp from Property");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t equipmentId = result->get_integer(i, "EquipmentId").second;
        vector<PropertyData> &theList = retVal[equipmentId];
        PropertyData entry;
        entry.name = result->get_string(i, "Name").second;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.inOperation = result->get_integer(i, "InOp").second;
        entry.outOperation = result->get_integer(i, "OutOp").second;
        theList.push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ScheduleRuleConditionData>> ConfigStorage::get_schedule_condition_data()
{
    unordered_map<uint16_t, vector<ConfigStorage::ScheduleRuleConditionData>> retVal;
    auto result = theDb.execute_query("Select ConditionId, Subject, Comparison, Value from ScheduleCondition");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t conditionId = result->get_integer(i, "ConditionId").second;
        vector<ScheduleRuleConditionData> &theList = retVal[conditionId];
        ScheduleRuleConditionData entry;
        entry.subject = result->get_string(i, "Subject").second;
        entry.comparison = result->get_string(i, "Comparison").second;
        entry.value = result->get_integer(i, "Value").second;
        theList.push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<uint16_t>> ConfigStorage::get_schedule_rule_and_condition_data()
{
    unordered_map<uint16_t, vector<uint16_t>> retVal;
    auto result = theDb.execute_query("Select RuleId, ConditionId from ScheduleRule");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t ruleId = result->get_integer(i, "RuleId").second;
        vector<uint16_t> &theList = retVal[ruleId];
        uint16_t scheduleId = result->get_integer(i, "ConditionId").second;
        theList.push_back(scheduleId);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ScheduleData>> ConfigStorage::get_schedule_data()
{
    unordered_map<uint16_t, vector<ScheduleData>> retVal;
    auto result = theDb.execute_query("Select ScheduleId, RuleId, TimeTableId, Priority from Schedule");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t ruleId = result->get_integer(i, "ScheduleId").second;
        vector<ScheduleData> &theList = retVal[ruleId];
        ScheduleData entry;
        entry.ruleId = result->get_integer(i, "RuleId").second;
        entry.timeTableId = result->get_integer(i, "TimeTableId").second;
        entry.priority = result->get_integer(i, "Priority").second;
        theList.push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::TimeTableData>> ConfigStorage::get_time_table_data()
{
    unordered_map<uint16_t, vector<TimeTableData>> retVal;
    auto result = theDb.execute_query("Select TimeTableId, EventType, Hour, Minute, Second, Value from TimeTable");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t timeTableId = result->get_integer(i, "TimeTableId").second;
        vector<TimeTableData> &theList = retVal[timeTableId];
        TimeTableData entry;
        entry.eventType = result->get_string(i, "EventType").second;
        entry.hour = result->get_integer(i, "Hour").second;
        entry.minute = result->get_integer(i, "Minute").second;
        entry.second = result->get_integer(i, "Second").second;
        entry.valueString = result->get_string(i, "Value").second;
        theList.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::EquipmentScheduleData> ConfigStorage::get_equipment_schedule_data()
{
    vector<EquipmentScheduleData> retVal;
    auto result = theDb.execute_query("Select EquipmentId, PropertyName, ScheduleId from ScheduleEquipment");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        EquipmentScheduleData entry;
        entry.equipmentId = result->get_integer(i, "EquipmentId").second;
        entry.propertyName = result->get_string(i, "PropertyName").second;
        entry.scheduleId = result->get_integer(i, "ScheduleId").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::AlarmLogicData> ConfigStorage::get_alarm_logic()
{
    vector<AlarmLogicData> retVal;
    auto result = theDb.execute_query("Select Id, Comparison, Value, Message, State, Code from AlarmLogic");
    for(size_t i=0; i<result->get_row_count(); ++i)
    {
        AlarmLogicData entry;
        entry.id = result->get_integer(i, "Id").second;
        entry.comparison = result->get_integer(i, "Comparison").second;
        entry.referenceValue = result->get_float(i, "Value").second;
        entry.message = result->get_string(i, "Message").second;
        entry.state = result->get_integer(i, "State").second;
        entry.code = result->get_integer(i, "Code").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::NodeAlarmData> ConfigStorage::get_node_alarm()
{
    vector<NodeAlarmData> retVal;
    auto result = theDb.execute_query("Select Equipment, Property, AlarmLogicId, Priority from NodeAlarm");
    for(size_t i=0; i<result->get_row_count(); ++i)
    {
        NodeAlarmData entry;
        entry.equipmentId.from_formatted_string(result->get_string(i, "Equipment").second);
        entry.propertyId.from_formatted_string(result->get_string(i, "Property").second);
        entry.alarmLogicId = result->get_integer(i, "AlarmLogicId").second;
        entry.priority = result->get_integer(i, "Priority").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::NodeNormalMessage> ConfigStorage::get_normal_message()
{
    vector<NodeNormalMessage> retVal;
    auto result = theDb.execute_query("Select Equipment, Property, Message from NodeNormalMessage");
    for(size_t i=0; i<result->get_row_count(); ++i)
    {
        NodeNormalMessage entry;
        entry.equipmentId.from_formatted_string(result->get_string(i, "Equipment").second);
        entry.propertyId.from_formatted_string(result->get_string(i, "Property").second);
        entry.message = result->get_string(i, "Message").second;
        retVal.push_back(entry);
    }
    return retVal;
}
