#include "ChannelManager.h"

using namespace std;

ChannelManager::ChannelManager(ConfigStorage &configStorage, SerialPortManager &serialPortManager, OpStorage &_opStorage) : opStorage(_opStorage)
{
    auto modbusIpData = configStorage.get_modbus_ip_channel_data();
    auto modbusRtuData = configStorage.get_modbus_rtu_channel_data();
    for (auto i = modbusIpData.begin(); i != modbusIpData.end(); ++i)
    {
        auto entry = make_unique<ModbusIpProcess>(i->hisAddress, i->hisPort, 1, i->maxRegister, i->maxCoil, !(i->bigEndian), std::chrono::milliseconds(i->msTimeout));
        modbusIpMap.emplace(i->deviceId, move(entry));
    }
    for (auto i = modbusRtuData.begin(); i != modbusRtuData.end(); ++i)
    {
        auto serialPort = serialPortManager.get_serial_port(i->serialPortName);
        if (serialPort != nullptr)
        {
            auto entry = make_unique<ModbusRtuProcess>(serialPort, i->slaveId, i->maxRegister, i->maxCoil, std::chrono::milliseconds(i->msTimeout), !(i->bigEndian));
            modbusRtuMap.emplace(i->deviceId, move(entry));
        }
    }
    printf("ChannelManager 1.\n");
    auto modbusIpPoint = configStorage.get_modbus_ip_point();
    for (auto i = modbusIpPoint.begin(); i != modbusIpPoint.end(); ++i)
    {
        //Make sure all points have a valid device id
        if (!modbusIpMap.count(i->first))
        {
            continue;
        }
        unordered_map<uint16_t, ModbusPointData> &channelEntry = modbusPoint[i->first];
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            if (GlobalEnum::is_modbus_data_type(i->second[j].type))
            {
                ModbusPointData &pointEntry = channelEntry[i->second[j].pointId];
                pointEntry.address = i->second[j].address;
                pointEntry.type = static_cast<GlobalEnum::ModbusDataType>(i->second[j].type);
                pointEntry.inOpertation = i->second[j].inOp;
                pointEntry.outOperation = i->second[j].outOp;
            }
            else
            {
                printf("Got invalid modbus data type %u at device %u point %u modbus ip, skipping\n", i->second[j].type, i->first, i->second[j].pointId);
            }
        }
    }
    printf("ChannelManager 2.\n");
    auto modbusRtuPoint = configStorage.get_modbus_rtu_point();
    for (auto i = modbusRtuPoint.begin(); i != modbusRtuPoint.end(); ++i)
    {
        //Make sure all points have a valid device id
        if (!modbusRtuMap.count(i->first))
        {
            continue;
        }
        unordered_map<uint16_t, ModbusPointData> &channelEntry = modbusPoint[i->first];
        for (size_t j = 0; j < i->second.size(); ++j)
        {
            if (GlobalEnum::is_modbus_data_type(i->second[j].type))
            {
                ModbusPointData &pointEntry = channelEntry[i->second[j].pointId];
                pointEntry.address = i->second[j].address;
                pointEntry.type = static_cast<GlobalEnum::ModbusDataType>(i->second[j].type);
                pointEntry.inOpertation = i->second[j].inOp;
                pointEntry.outOperation = i->second[j].outOp;
            }
            else
            {
                printf("Got invalid modbus data type %u at device %u point %u modbus rtu, skipping\n", i->second[j].type, i->first, i->second[j].pointId);
            }
        }
    }
}

ChannelManager::~ChannelManager()
{
}

shared_ptr<Variable> ChannelManager::get_point(uint16_t deviceId, uint16_t pointId)
{
    if (!modbusPoint.count(deviceId))
    {
        printf("Device %u not found.\n", deviceId);
        return nullptr;
    }
    unordered_map<uint16_t, ModbusPointData> &channelEntry = modbusPoint[deviceId];
    if (!channelEntry.count(pointId))
    {
        printf("Point %u not found.\n", pointId);
        return nullptr;
    }
    ModbusPointData &pointEntry = channelEntry[pointId];
    shared_ptr<Variable> retVal;
    if (modbusIpMap.count(deviceId))
    {
        if (GlobalEnum::is_modbus_coil_type(pointEntry.type))
        {
            retVal = modbusIpMap[deviceId]->get_coil_status_variable(pointEntry.address, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_digital_input_type(pointEntry.type))
        {
            retVal = modbusIpMap[deviceId]->get_digital_input_variable(pointEntry.address, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_holding_register_type(pointEntry.type))
        {
            retVal = modbusIpMap[deviceId]->get_holding_register_variable(pointEntry.address, pointEntry.type, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_input_register_type(pointEntry.type))
        {
            retVal = modbusIpMap[deviceId]->get_input_register_variable(pointEntry.address, pointEntry.type, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
    }
    else if (modbusRtuMap.count(deviceId))
    {
        if (GlobalEnum::is_modbus_coil_type(pointEntry.type))
        {
            retVal = modbusRtuMap[deviceId]->get_coil_status_variable(pointEntry.address, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_digital_input_type(pointEntry.type))
        {
            retVal = modbusRtuMap[deviceId]->get_digital_input_variable(pointEntry.address, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_holding_register_type(pointEntry.type))
        {
            retVal = modbusRtuMap[deviceId]->get_holding_register_variable(pointEntry.address, pointEntry.type, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
        else if (GlobalEnum::is_modbus_input_register_type(pointEntry.type))
        {
            retVal = modbusRtuMap[deviceId]->get_input_register_variable(pointEntry.address, pointEntry.type, opStorage.get_logic(pointEntry.inOpertation), opStorage.get_logic(pointEntry.outOperation));
        }
    }
    return retVal;
}

void ChannelManager::attach_to_tree(shared_ptr<VariableTree> theTree)
{
    for (auto channel = modbusPoint.begin(); channel != modbusPoint.end(); ++channel)
    {
        shared_ptr<VariableTree> channelBranch = make_shared<VariableTree>();
        for (auto point = channel->second.begin(); point != channel->second.end(); ++point)
        {
            channelBranch->create_leaf(point->first, this->get_point(channel->first, point->first));
        }
        theTree->add_child(channel->first, channelBranch);
    }
}

void ChannelManager::start()
{
    for (auto i = modbusIpMap.begin(); i != modbusIpMap.end(); ++i)
    {
        i->second->start();
    }
    for (auto i = modbusRtuMap.begin(); i != modbusRtuMap.end(); ++i)
    {
        i->second->start();
    }
}
