#include "Builder.h"
#include "../../MyLib/Basic/Helper.hpp"

using namespace std;

Builder::Builder(const string& dbName) : theDb(dbName) {

}

Builder::~Builder() {

}

vector<size_t> Builder::get_modbus_ip_device_id() {
    vector<size_t> retVal;
    auto result = theDb.execute_query("Select Id from PeripheralDevice where Type==2");
    for(size_t i=0; i<result->get_row_count(); ++i) {
        retVal.push_back(result->get_integer(i, "Id").second);        
    }
    return retVal;
}

std::vector<size_t> Builder::get_modbus_rtu_device_id() {
    vector<size_t> retVal;
    auto result = theDb.execute_query("Select Id from PeripheralDevice where Type==1");
    for(size_t i=0; i<result->get_row_count(); ++i) {
        retVal.push_back(result->get_integer(i, "Id").second);        
    }
    return retVal;
}

unordered_map<size_t, unique_ptr<ModbusIpProcess>> Builder::get_modbus_ip_process(const std::vector<size_t> deviceId) {
    unordered_map<size_t, unique_ptr<ModbusIpProcess>> retVal;
    for(auto& entry : deviceId) {
        auto result = theDb.execute_query("Select SlaveAddress, SlavePort, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusIpSlave where DeviceId==%u", entry);
        if(!result->get_row_count()) {
            continue;
        }
        string hisAddress = result->get_string(0, "SlaveAddress").second;
        uint16_t hisPort = result->get_integer(0, "SlavePort").second;
        bool bigEndian = result->get_integer(0, "BigEndian").second;
        uint16_t maxRegister = result->get_integer(0, "MaxRegister").second;
        uint16_t maxCoil = result->get_integer(0, "MaxCoil").second;
        uint16_t timeout = result->get_integer(0, "Timeout_ms").second;
        unique_ptr<ModbusIpProcess> newProcess = make_unique<ModbusIpProcess>(hisAddress, hisPort, 1, maxRegister, maxCoil, !bigEndian, std::chrono::milliseconds(timeout));
        retVal.emplace(entry, move(newProcess));
    }
    return retVal;
}

shared_ptr<SyncedSerialPort> create_serial_port(Sqlite3& theDb, const string& serialPortName) {
    auto result = theDb.execute_query("Select Baudrate, ParityBit, StopBit, HardwareFlowControl, SoftwareFlowControl, BitPerByte, Delay_ms from SerialPortConfig where Name=='%s'", serialPortName.c_str());
    SyncedSerialPort::Config config;
    if(!result->get_row_count()) {
        return nullptr;
    }
    config.baudrate = result->get_integer(0, "Baudrate").second;
    config.parityBit = result->get_integer(0, "ParityBit").second;
    config.stopBit = result->get_integer(0, "StopBit").second;
    config.hardwareFlowControl = result->get_integer(0, "HardwareFlowControl").second;
    config.softwareFlowControl = result->get_integer(0, "SoftwareFlowControl").second;
    config.bitPerByte = result->get_integer(0, "BitPerByte").second;
    uint32_t delayMs = result->get_integer(0, "Delay_ms").second;
    shared_ptr<SyncedSerialPort> serialPort = make_shared<SyncedSerialPort>();
    serialPort->open(serialPortName, config);
    serialPort->set_delay(chrono::milliseconds(delayMs));
    return serialPort;
}

unordered_map<size_t, unique_ptr<ModbusRtuProcess>> Builder::get_modbus_rtu_process(const std::vector<size_t> deviceId) {
    unordered_map<size_t, unique_ptr<ModbusRtuProcess>> retVal;
    unordered_map<string, shared_ptr<SyncedSerialPort>> serialPortMap;
    for(auto& entry : deviceId) {
        auto result = theDb.execute_query("Select SerialPortName, SlaveId, BigEndian, MaxRegister, MaxCoil, Timeout_ms from ModbusRtuSlave where DeviceId==%u", entry);
        if(!result->get_row_count()) {
            continue;
        }
        string serialPortName = result->get_string(0, "SerialPortName").second;
        if(!serialPortMap.count(serialPortName)) {
            serialPortMap.emplace(serialPortName, create_serial_port(theDb, serialPortName));
        }
        auto serialPort = serialPortMap[serialPortName];
        if(serialPort == nullptr) {
            continue;
        }
        uint8_t slaveId = result->get_integer(0, "SlaveId").second;
        bool bigEndian = result->get_integer(0, "BigEndian").second;
        uint16_t maxRegister = result->get_integer(0, "MaxRegister").second;
        uint16_t maxCoil = result->get_integer(0, "MaxCoil").second;
        uint16_t timeout = result->get_integer(0, "Timeout_ms").second;
        unique_ptr<ModbusRtuProcess> newProcess = make_unique<ModbusRtuProcess>(serialPort, slaveId, maxRegister, maxCoil, chrono::milliseconds(timeout), !bigEndian);
        retVal.emplace(entry, move(newProcess));
    }
    return retVal;
}

unordered_map<size_t, shared_ptr<Variable>> Builder::get_modbus_ip_variables(size_t deviceId, unique_ptr<ModbusIpProcess>& modbusIp) {
    unordered_map<size_t, shared_ptr<Variable>> retVal;
    auto result = theDb.execute_query("Select PointId, Address, Type from ModbusIpPoint where DeviceId==%u", deviceId);
    for(size_t i=0; i<result->get_row_count(); ++i) {
        size_t pointId = result->get_integer(i, "PointId").second;
        uint16_t address = result->get_integer(i, "Address").second;
        ModbusRegisterValue::DataType type = ModbusRegisterValue::DataType(result->get_integer(i, "Type").second);
        shared_ptr<Variable> element;
        if(type == ModbusRegisterValue::DataType::COIL) {
            element = modbusIp->get_coil_status_variable(address);
        }
        else {
            element = modbusIp->get_holding_register_variable(address, type);
        }
        retVal.emplace(pointId, element);
    }
    return retVal;
}

unordered_map<size_t, shared_ptr<Variable>> Builder::get_modbus_rtu_variables(size_t deviceId, unique_ptr<ModbusRtuProcess>& modbusRtu) {
    unordered_map<size_t, shared_ptr<Variable>> retVal;
    auto result = theDb.execute_query("Select PointId, Address, Type from ModbusRtuPoint where DeviceId==%u", deviceId);
    for(size_t i=0; i<result->get_row_count(); ++i) {
        size_t pointId = result->get_integer(i, "PointId").second;
        uint16_t address = result->get_integer(i, "Address").second;
        ModbusRegisterValue::DataType type = ModbusRegisterValue::DataType(result->get_integer(i, "Type").second);
        shared_ptr<Variable> element;
        if(type == ModbusRegisterValue::DataType::COIL) {
            element = modbusRtu->create_coil_status_variable(address);
        }
        else {
            element = modbusRtu->create_holding_register_variable(address, type);
        }
        retVal.emplace(pointId, element);
    }
    return retVal;
}

AlarmDefinition::Comparison string_to_comparison(const string& input) {
    if(!input.compare(">")) {
        return AlarmDefinition::Comparison::GREATER;
    }
    if(!input.compare(">=")) {
        return AlarmDefinition::Comparison::GREATER_EQUAL;
    }
    if(!input.compare("==")) {
        return AlarmDefinition::Comparison::EQUAL;
    }
    if(!input.compare("!=")) {
        return AlarmDefinition::Comparison::NOT_EQUAL;
    }
    if(!input.compare("<=")) {
        return AlarmDefinition::Comparison::SMALLER_EQUAL;
    }
    if(!input.compare("<")) {
        return AlarmDefinition::Comparison::SMALLER;
    }
    throw 123;
}

AlarmDefinition::ConditionType string_to_type(const string& input) {
    if(!input.compare("Alarm")) {
        return AlarmDefinition::ConditionType::Alarm;
    }
    if(!input.compare("Event")) {
        return AlarmDefinition::ConditionType::Event;
    }
    if(!input.compare("Error")) {
        return AlarmDefinition::ConditionType::Error;
    }
    return AlarmDefinition::ConditionType::None;
}

unordered_map<int32_t, AlarmDefinition::AlarmLogicConstant> Builder::get_logic() {
    unordered_map<int32_t, AlarmDefinition::AlarmLogicConstant> retVal;
    auto result = theDb.execute_query("Select Id, Compare, Value, Message, Type, Code from AlarmLogic");
    if(result != nullptr) {
        for(size_t i=0; i<result->get_row_count(); ++i) {
            int32_t id = result->get_integer(i, "Id").second;
            string sCompare = result->get_string(i, "Compare").second;
            AlarmDefinition::Comparison compare;
            try {
                compare = string_to_comparison(sCompare);
            } catch(int e) { continue; }
            Value value = result->get_float(i, "Value").second;
            string message = result->get_string(i, "Message").second;
            string sType = result->get_string(i, "Type").second;
            AlarmDefinition::ConditionType type = string_to_type(sType);
            uint16_t code = result->get_integer(i, "Code").second;
            retVal.emplace(id, AlarmDefinition::AlarmLogicConstant{compare, value, {type, code}, message});
            //printf("Value: %s\n", retVal[id].rightValue.to_string().c_str());
        }
    }
    return retVal;
}

unordered_map<HashKey::DualKey, vector<Builder::LogicInfo>, HashKey::DualKeyHash> Builder::get_logic_pair() {
    unordered_map<HashKey::DualKey, vector<Builder::LogicInfo>, HashKey::DualKeyHash> retVal;
    auto result = theDb.execute_query("Select Equipment, Property, AlarmLogicId, Priority from PropertyAlarm");
    if(result != nullptr) {
        for(size_t i=0; i<result->get_row_count(); ++i) {
            string sEquipment = result->get_string(i, "Equipment").second;
            string sProperty = result->get_string(i, "Property").second;
            uint32_t logicId = result->get_integer(i, "AlarmLogicId").second;
            uint8_t priority = result->get_integer(i, "Priority").second;
            HashKey::EitherKey equipment;
            equipment.from_formatted_string(sEquipment);
            HashKey::EitherKey property;
            property.from_formatted_string(sProperty);
            //retVal.emplace(HashKey::DualKey{equipment, property}, Builder::LogicInfo{logicId, priority});
            retVal[HashKey::DualKey{equipment, property}].push_back(Builder::LogicInfo{logicId, priority});
        }
    }
    return retVal;
}