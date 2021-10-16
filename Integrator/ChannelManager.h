#ifndef _ChannelManager_H_
#define _ChannelManager_H_
#include <memory>
#include <unordered_map>

#include "../Storage/ConfigStorage.h"
#include "../Modbus/ModbusIpProcess.h"
#include "../Modbus/ModbusRtuProcess.h"
#include "../VariableTree/VariableTree.h"
#include "SerialPortManager.h"

//Channel Manager manages channels, physical peripheral devices

class ChannelManager {
public:
    ChannelManager(ConfigStorage& configStorage, SerialPortManager& serialPortManager);
    virtual ~ChannelManager();
    
    std::shared_ptr<Variable> get_point(uint16_t deviceId, uint16_t pointId);
    void attach_to_tree(std::shared_ptr<VariableTree> theTree);
    void start();

private:
    std::unordered_map<uint16_t, std::unique_ptr<ModbusIpProcess>> modbusIpMap;
    std::unordered_map<uint16_t, std::unique_ptr<ModbusRtuProcess>> modbusRtuMap;

    struct ModbusPointData {
        uint16_t address;
        ModbusRegisterValue::DataType type;
    };
    std::unordered_map<uint16_t, std::unordered_map<uint16_t, ModbusPointData>> modbusPoint;
};

#endif