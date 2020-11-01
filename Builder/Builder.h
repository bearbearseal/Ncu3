
//Builder add builder for each type of channel type
//Call channel builder to build channels
//Call channels to build points

#ifndef _Builder_Builder_H_
#define _Builder_Builder_H_
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../../MyLib/Sqlite/Sqlite3.h"
#include "../../MyLib/Basic/HashKey.h"
#include "../../MyLib/Basic/Variable.h"
#include "../Modbus/ModbusIpProcess.h"
#include "../Modbus/ModbusRtuProcess.h"
#include "../Alarm/AlarmDefinition.h"

class Builder {
public:
    Builder(const std::string& dbName);
    ~Builder();

    std::vector<size_t> get_modbus_ip_device_id();
    std::vector<size_t> get_modbus_rtu_device_id();

    std::unordered_map<size_t, std::unique_ptr<ModbusIpProcess>> get_modbus_ip_process(const std::vector<size_t> deviceId);
    std::unordered_map<size_t, std::unique_ptr<ModbusRtuProcess>> get_modbus_rtu_process(const std::vector<size_t> deviceId);

    std::unordered_map<size_t, std::shared_ptr<Variable>> get_modbus_ip_variables(size_t deviceId, std::unique_ptr<ModbusIpProcess>& modbusIp);
    std::unordered_map<size_t, std::shared_ptr<Variable>> get_modbus_rtu_variables(size_t deviceId, std::unique_ptr<ModbusRtuProcess>& modbusRtu);

    struct LogicInfo {
        uint32_t logicId;
        uint16_t priority;
    };
    std::unordered_map<int32_t, AlarmDefinition::AlarmLogicConstant> get_logic();
    std::unordered_map<HashKey::DualKey, std::vector<LogicInfo>, HashKey::DualKeyHash> get_logic_pair();

private:
    Sqlite3 theDb;
};

#endif