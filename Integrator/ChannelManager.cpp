#include "ChannelManager.h"

using namespace std;

ChannelManager::ChannelManager(ConfigStorage &configStorage, SerialPortManager& serialPortManager)
{
    auto modbusIpData = configStorage.get_modbus_ip_channel_data();
    auto modbusRtuData = configStorage.get_modbus_rtu_channel_data();
    for (auto i = modbusIpData.begin(); i != modbusIpData.end(); ++i)
    {
        auto entry = make_unique<ModbusIpProcess>(i->hisAddress, i->hisPort, 1, i->maxRegister, i->maxCoil, !(i->bigEndian), std::chrono::milliseconds(i->msTimeout));
        modbusIpMap.emplace(i->deviceId, move(entry));
    }
    for(auto i=modbusRtuData.begin(); i!=modbusRtuData.end(); ++i) {
        auto serialPort = serialPortManager.get_serial_port(i->serialPortName);
        if(serialPort != nullptr) {
            auto entry = make_unique<ModbusRtuProcess>(serialPort, i->slaveId, i->maxRegister, i->maxCoil, std::chrono::milliseconds(i->msTimeout), !(i->bigEndian));
            modbusRtuMap.emplace(i->deviceId, move(entry));
        }
    }
    auto modbusIpPoint = configStorage.get_modbus_ip_point();
    auto modbusRtuPoint = configStorage.get_modbus_rtu_point();
    for(auto i=modbusIpPoint.begin(); i != modbusIpPoint.end(); ++i) {
        //Make sure all points have a valid device id
        if(!modbusIpMap.count(i->first)) {
            continue;
        }
        unordered_map<uint16_t, ModbusPointData>& channelEntry = modbusPoint[i->first];
        for(size_t j=0; j<i->second.size(); ++j) {
            ModbusPointData& pointEntry = channelEntry[i->second[j].pointId];
            pointEntry.address = i->second[j].address;
            pointEntry.type = ModbusRegisterValue::convert_integer_to_data_type(i->second[j].type);
            if(pointEntry.type == ModbusRegisterValue::DataType::UNKNOWN) {
                pointEntry.type = ModbusRegisterValue::DataType::COIL;
            }
        }
    }
    for(auto i=modbusRtuPoint.begin(); i != modbusRtuPoint.end(); ++i) {
        //Make sure all points have a valid device id
        if(!modbusRtuMap.count(i->first)) {
            continue;
        }
        unordered_map<uint16_t, ModbusPointData>& channelEntry = modbusPoint[i->first];
        for(size_t j=0; j<i->second.size(); ++j) {
            ModbusPointData& pointEntry = channelEntry[i->second[j].pointId];
            pointEntry.address = i->second[j].address;
            pointEntry.type = ModbusRegisterValue::convert_integer_to_data_type(i->second[j].type);
            if(pointEntry.type == ModbusRegisterValue::DataType::UNKNOWN) {
                pointEntry.type = ModbusRegisterValue::DataType::COIL;
            }
        }
    }
}

ChannelManager::~ChannelManager()
{
}

shared_ptr<Variable> ChannelManager::get_point(uint16_t deviceId, uint16_t pointId)
{
    if(!modbusPoint.count(deviceId)) {
        printf("Device %u not found.\n", deviceId);
        return nullptr;
    }
    unordered_map<uint16_t, ModbusPointData>& channelEntry = modbusPoint[deviceId];
    if(!channelEntry.count(pointId)) {
        printf("Point %u not found.\n", pointId);
        return nullptr;
    }
    ModbusPointData& pointEntry = channelEntry[pointId]; 
    shared_ptr<Variable> retVal;
    if(modbusIpMap.count(deviceId)) {
        if(pointEntry.type == ModbusRegisterValue::DataType::COIL) {
            retVal = modbusIpMap[deviceId]->get_coil_status_variable(pointEntry.address);
        }
        else {
            retVal = modbusIpMap[deviceId]->get_holding_register_variable(pointEntry.address, pointEntry.type);
        }
    }
    else if(modbusRtuMap.count(deviceId)) {
        if(pointEntry.type == ModbusRegisterValue::DataType::COIL) {
            retVal = modbusRtuMap[deviceId]->create_coil_status_variable(pointEntry.address);
        }
        else {
            retVal = modbusRtuMap[deviceId]->create_holding_register_variable(pointEntry.address, pointEntry.type);
        }
    }
    return retVal;
}

void ChannelManager::start()
{
    for(auto i=modbusIpMap.begin(); i != modbusIpMap.end(); ++i) {
        i->second->start();
    }
    for(auto i=modbusRtuMap.begin(); i != modbusRtuMap.end(); ++i) {
        i->second->start();
    }
}
