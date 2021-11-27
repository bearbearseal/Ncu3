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
    // printf("Got %zu modbus ip channel data.\n", retVal.size());
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ModbusIpPoint>> ConfigStorage::get_modbus_ip_point()
{
    unordered_map<uint16_t, vector<ModbusIpPoint>> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type, InOp, OutOp from ModbusIpPoint");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusIpPoint entry;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
        entry.inOp = result->get_integer(i, "InOp").second;
        entry.outOp = result->get_integer(i, "OutOp").second;
        // entry.alarmLogic = result->get_integer(i, "AlarmLogic").second;
        retVal[deviceId].push_back(entry);
    }
    return retVal;
}

unordered_map<uint16_t, vector<ConfigStorage::ModbusRtuPoint>> ConfigStorage::get_modbus_rtu_point()
{
    unordered_map<uint16_t, vector<ModbusRtuPoint>> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type, InOp, OutOp from ModbusRtuPoint");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusRtuPoint entry;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
        entry.inOp = result->get_integer(i, "InOp").second;
        entry.outOp = result->get_integer(i, "OutOp").second;
        // entry.alarmLogic = result->get_integer(i, "AlarmLogic").second;
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
/*
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
*/
vector<ConfigStorage::RuleTableData> ConfigStorage::get_schedule()
{
    vector<ConfigStorage::RuleTableData> retVal;
    auto result = theDb.execute_query("Select Id, Rule1, TimeTable1, Rule2, TimeTable2, Rule3, TimeTable3, "
                                      "Rule4, TimeTable4, DefaultTable from Schedule");
    retVal.resize(result->get_row_count());
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        retVal[i].id = result->get_integer(i, "Id").second;
        retVal[i].pairList.resize(4);
        retVal[i].pairList[0].scheduleRule = result->get_integer(i, "Rule1").second;
        retVal[i].pairList[0].timeTable = result->get_integer(i, "TimeTable1").second;
        retVal[i].pairList[1].scheduleRule = result->get_integer(i, "Rule2").second;
        retVal[i].pairList[1].timeTable = result->get_integer(i, "TimeTable2").second;
        retVal[i].pairList[2].scheduleRule = result->get_integer(i, "Rule3").second;
        retVal[i].pairList[2].timeTable = result->get_integer(i, "TimeTable3").second;
        retVal[i].pairList[3].scheduleRule = result->get_integer(i, "Rule4").second;
        retVal[i].pairList[3].timeTable = result->get_integer(i, "TimeTable4").second;
    }
    return retVal;
}

unordered_map<uint32_t, vector<vector<ConfigStorage::ScheduleRule>>> ConfigStorage::get_schedule_rule()
{
    unordered_map<uint32_t, vector<vector<ScheduleRule>>> retVal;
    auto result = theDb.execute_query("Select `Group`, Subject1, Compare1, Value1, Subject2, Compare2, Value2, "
                                      "Subject3, Compare3, Value3, Subject4, Compare4, Value4 from ScheduleRule");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        vector<vector<ScheduleRule>> &mapEntry = retVal[result->get_integer(i, "Group").second];
        // Each row in sqlite table has 4 rules
        vector<ScheduleRule> ruleSubList(4);
        ruleSubList[0].subject = result->get_integer(i, "Subject1").second;
        ruleSubList[0].compare = result->get_integer(i, "Compare1").second;
        ruleSubList[0].value = result->get_integer(i, "Value1").second;
        ruleSubList[1].subject = result->get_integer(i, "Subject2").second;
        ruleSubList[1].compare = result->get_integer(i, "Compare2").second;
        ruleSubList[1].value = result->get_integer(i, "Value2").second;
        ruleSubList[2].subject = result->get_integer(i, "Subject3").second;
        ruleSubList[2].compare = result->get_integer(i, "Compare3").second;
        ruleSubList[2].value = result->get_integer(i, "Value3").second;
        ruleSubList[3].subject = result->get_integer(i, "Subject4").second;
        ruleSubList[3].compare = result->get_integer(i, "Compare4").second;
        ruleSubList[3].value = result->get_integer(i, "Value4").second;
        mapEntry.push_back(ruleSubList);
    }
    return retVal;
}

vector<ConfigStorage::TimeTableData> ConfigStorage::get_time_table()
{
    vector<TimeTableData> retVal;
    auto result = theDb.execute_query("Select Id, Time1, Action1, Value1, Time2, Action2, Value2, Time3, Action3, Value3, "
                                      "Time4, Action4, Value4, Time5, Action5, Value5, Time6, Action6, Value6, "
                                      "Time7, Action7, Value7, Time8, Action8, Value8 from TimeTable");
    retVal.resize(result->get_row_count());
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        retVal[i].id = result->get_integer(i, "Id").second;
        retVal[i].actionList.resize(8);
        retVal[i].actionList[0].time = result->get_integer(i, "Time1").second;
        retVal[i].actionList[0].action = result->get_integer(i, "Action1").second;
        retVal[i].actionList[0].value = result->get_float(i, "Value1").second;
        retVal[i].actionList[1].time = result->get_integer(i, "Time2").second;
        retVal[i].actionList[1].action = result->get_integer(i, "Action2").second;
        retVal[i].actionList[1].value = result->get_float(i, "Value2").second;
        retVal[i].actionList[2].time = result->get_integer(i, "Time3").second;
        retVal[i].actionList[2].action = result->get_integer(i, "Action3").second;
        retVal[i].actionList[2].value = result->get_float(i, "Value3").second;
        retVal[i].actionList[3].time = result->get_integer(i, "Time4").second;
        retVal[i].actionList[3].action = result->get_integer(i, "Action4").second;
        retVal[i].actionList[3].value = result->get_float(i, "Value4").second;
        retVal[i].actionList[4].time = result->get_integer(i, "Time5").second;
        retVal[i].actionList[4].action = result->get_integer(i, "Action5").second;
        retVal[i].actionList[4].value = result->get_float(i, "Value5").second;
        retVal[i].actionList[5].time = result->get_integer(i, "Time6").second;
        retVal[i].actionList[5].action = result->get_integer(i, "Action6").second;
        retVal[i].actionList[5].value = result->get_float(i, "Value6").second;
        retVal[i].actionList[6].time = result->get_integer(i, "Time7").second;
        retVal[i].actionList[6].action = result->get_integer(i, "Action7").second;
        retVal[i].actionList[6].value = result->get_float(i, "Value7").second;
        retVal[i].actionList[7].time = result->get_integer(i, "Time8").second;
        retVal[i].actionList[7].action = result->get_integer(i, "Action8").second;
        retVal[i].actionList[7].value = result->get_float(i, "Value8").second;
    }
    return retVal;
}

vector<ConfigStorage::AlarmLogicsData> ConfigStorage::get_alarm_logic()
{
    vector<AlarmLogicsData> retVal;
    auto result = theDb.execute_query("Select Id, Compare1, RefValue1, State1, "
                                      "Compare2, RefValue2, State2, "
                                      "Compare3, RefValue3, State3, "
                                      "Compare4, RefValue4, State4, "
                                      "Compare5, RefValue5, State5, "
                                      "Compare6, RefValue6, State6 from AlarmLogic");
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        AlarmLogicsData entry;
        entry.logicData.resize(6);
        entry.id = result->get_integer(i, "Id").second;
        entry.logicData[0].compare = result->get_integer(i, "Compare1").second;
        entry.logicData[0].refValue = result->get_float(i, "RefValue1").second;
        entry.logicData[0].state = result->get_integer(i, "State1").second;
        entry.logicData[1].compare = result->get_integer(i, "Compare2").second;
        entry.logicData[1].refValue = result->get_float(i, "RefValue2").second;
        entry.logicData[1].state = result->get_integer(i, "State2").second;
        entry.logicData[2].compare = result->get_integer(i, "Compare3").second;
        entry.logicData[2].refValue = result->get_float(i, "RefValue3").second;
        entry.logicData[2].state = result->get_integer(i, "State3").second;
        entry.logicData[3].compare = result->get_integer(i, "Compare4").second;
        entry.logicData[3].refValue = result->get_float(i, "RefValue4").second;
        entry.logicData[3].state = result->get_integer(i, "State4").second;
        entry.logicData[4].compare = result->get_integer(i, "Compare5").second;
        entry.logicData[4].refValue = result->get_float(i, "RefValue5").second;
        entry.logicData[4].state = result->get_integer(i, "State5").second;
        entry.logicData[5].compare = result->get_integer(i, "Compare6").second;
        entry.logicData[5].refValue = result->get_float(i, "RefValue6").second;
        entry.logicData[5].state = result->get_integer(i, "State6").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::PointSchedulePair> ConfigStorage::get_point_schedule_pair()
{
    vector<PointSchedulePair> retVal;
    auto result = theDb.execute_query("Select Device, Point, Schedule from SchedulePointPair");
    retVal.resize(result->get_row_count());
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        retVal[i].deviceId = result->get_integer(i, "Device").second;
        retVal[i].pointId = result->get_integer(i, "Point").second;
        retVal[i].scheduleId = result->get_integer(i, "Schedule").second;
    }
    return retVal;
}

vector<ConfigStorage::NodeAlarmData> ConfigStorage::get_node_alarm()
{
    vector<NodeAlarmData> retVal;
    auto result = theDb.execute_query("Select Equipment, Property, AlarmLogicId, Priority from NodeAlarm");
    for (size_t i = 0; i < result->get_row_count(); ++i)
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
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        NodeNormalMessage entry;
        entry.equipmentId.from_formatted_string(result->get_string(i, "Equipment").second);
        entry.propertyId.from_formatted_string(result->get_string(i, "Property").second);
        entry.message = result->get_string(i, "Message").second;
        retVal.push_back(entry);
    }
    return retVal;
}

vector<ConfigStorage::PointAlarmPair> ConfigStorage::get_alarm_point_pair()
{
    vector<PointAlarmPair> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, LogicGroupId from PointAlarmPair");
    retVal.resize(result->get_row_count());
    for (size_t i = 0; i < result->get_row_count(); ++i)
    {
        retVal[i].deviceId = result->get_integer(i, "DeviceId").second;
        retVal[i].pointId = result->get_integer(i, "PointId").second;
        retVal[i].logicGroupId = result->get_integer(i, "LogicGroupId").second;
    }
    return retVal;
}
