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
    for(size_t i=0; i<result->get_column_count(); ++i) {
        string name = result->get_string(i, "Name").second;
        SerialPortData& entry = retVal[name];
        entry.baudrate = result->get_integer(i, "Baudrate").second;
        entry.paritybit = result->get_integer(i, "ParityBit").second;
        entry.stopbit = result->get_integer(i, "StopBit").second;
        entry.hardwareFlowControl = result->get_integer(i, "HardwareFlowControl").second;
        entry.softwareFlowControl = result->get_integer(i, "SoftwareFlowControl").second;
        entry.bitPerByte = result->get_integer(i, "BitPerByte").second;
        entry.msDelay = result->get_integer(i, "Timeout_ms").second;
    }
    return retVal;
}

vector<ConfigStorage::ModbusIpChannelData> ConfigStorage::get_modbus_ip_channel_data()
{
    vector<ModbusIpChannelData> retVal;
    auto result = theDb.execute_query("Select DeviceId, SlaveAddress, SlavePort, TagName, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusIpSlave");
    for(size_t i=0; i<result->get_column_count(); ++i) {
        ModbusIpChannelData entry;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.hisAddress = result->get_string(i, "SlaveAddress").second;
        entry.hisPort = result->get_integer(i, "SlavePort").second;
        entry.tagName = result->get_string(i, "TagName").second;
        entry.bigEndian = result->get_integer(i, "BigEndian").second;
        entry.maxRegister = result->get_integer(i, "maxRegister").second;
        entry.maxCoil = result->get_integer(i, "MaxCoil").second;
        entry.msTimeout = result->get_integer(i, "Timeout_ms").second;
        retVal.push_back(move(entry));
    }
}

unordered_map<uint16_t, ConfigStorage::ModbusIpPoint> ConfigStorage::get_modbus_ip_point()
{
    unordered_map<uint16_t, ModbusIpPoint> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type from ModbusIpPoint");
    for(size_t i=0; i<result->get_column_count(); ++i) {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusIpPoint& entry = retVal[deviceId];
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
    } 
    return retVal;
}

unordered_map<uint16_t, ConfigStorage::ModbusRtuPoint> ConfigStorage::get_modbus_rtu_point()
{
    unordered_map<uint16_t, ModbusRtuPoint> retVal;
    auto result = theDb.execute_query("Select DeviceId, PointId, Address, Type from ModbusRtuPoint");
    for(size_t i=0; i<result->get_column_count(); ++i) {
        uint16_t deviceId = result->get_integer(i, "DeviceId").second;
        ModbusRtuPoint& entry = retVal[deviceId];
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.address = result->get_integer(i, "Address").second;
        entry.type = result->get_integer(i, "Type").second;
    } 
    return retVal;
}

vector<ConfigStorage::ModbusRtuChannelData> ConfigStorage::get_modbus_rtu_channel_data()
{
    vector<ModbusRtuChannelData> retVal;
    auto result = theDb.execute_query("Select DeviceId, SerialPortName, SlaveId, TagName, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusRtuSlave");
    for(size_t i=0; i< result->get_column_count(); ++i) {
        ModbusRtuChannelData entry;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.serialPortName = result->get_string(i, "SerialPortName").second;
        entry.slaveId = result->get_integer(i, "SlaveId").second;
        entry.tagName = result->get_string(i, "TagName").second;
        entry.bigEndian = result->get_integer(i, "BigEndian").second;
        entry.maxRegister = result->get_integer(i, "MaxRegister").second;
        entry.maxCoil = result->get_integer(i, "MaxCoil").second;
        entry.msTimeout = result->get_integer(i, "Timeout_ms").second;
        retVal.push_back(move(entry));
    }
    return retVal;
}

vector<ConfigStorage::EquipmentData> ConfigStorage::get_equipment_data()
{
    vector<EquipmentData> retVal;
    auto result = theDb.execute_query("Select Id, Name from Equipment");
    for(size_t i=0; i<result->get_column_count(); ++i) {
        EquipmentData entry;
        entry.equipmentId = result->get_integer(i, "Id").second;
        entry.name = result->get_string(i, "Name").second;
        retVal.push_back(move(entry));
    }
    return retVal;
}

unordered_map<uint16_t, ConfigStorage::PropertyData> ConfigStorage::get_property_data()
{
    unordered_map<uint16_t, PropertyData> retVal;
    auto result = theDb.execute_query("Select EquipmentId, Name, DeviceId, PointId, InOp, OutOp from Property");
    for(size_t i=0; i<result->get_column_count(); ++i) {
        uint16_t equipmentId = result->get_integer(i, "EquipmentId").second;
        PropertyData& entry = retVal[equipmentId];
        entry.name = result->get_string(i, "Name").second;
        entry.deviceId = result->get_integer(i, "DeviceId").second;
        entry.pointId = result->get_integer(i, "PointId").second;
        entry.inOperation = result->get_integer(i, "InOp").second;
        entry.outOperation = result->get_integer(i, "OutOp").second;
    }
}
