#include "VariableTree/VariableTree.h"
#include "VariableTree/TcpTalker.h"
#include "Builder/Builder.h"
#include <thread>

using namespace std;

namespace Deploy {
    void run_real_time_value() {
        shared_ptr<VariableTree> variableTree = make_shared<VariableTree>();
        unordered_map<size_t, unique_ptr<ModbusIpProcess>> modbusIpProcessMap;
        unordered_map<size_t, unique_ptr<ModbusRtuProcess>> modbusRtuProcessMap;
        unordered_map<size_t, vector<shared_ptr<Variable>>> allVariables;
        {
            Builder aBuilder("/var/sqlite/NcuConfig.db");
            auto modbusIpIdList = aBuilder.get_modbus_ip_device_id();
            modbusIpProcessMap = aBuilder.get_modbus_ip_process(modbusIpIdList);
            for(auto& i : modbusIpProcessMap) {
                unordered_map<size_t, shared_ptr<Variable>> variables = aBuilder.get_modbus_ip_variables(i.first, i.second);
                shared_ptr<VariableTree> branch = variableTree->create_branch(i.first);
                for(auto& j : variables) {
                    branch->create_leaf(j.first, j.second);
                    allVariables[i.first].push_back(j.second);
                }
            }
            auto modbusRtuList = aBuilder.get_modbus_rtu_device_id();
            modbusRtuProcessMap = aBuilder.get_modbus_rtu_process(modbusRtuList);
            for(auto& i : modbusRtuProcessMap) {
                unordered_map<size_t, shared_ptr<Variable>> variables = aBuilder.get_modbus_rtu_variables(i.first, i.second);
                shared_ptr<VariableTree> branch = variableTree->create_branch(i.first);
                for(auto& j : variables) {
                    branch->create_leaf(j.first, j.second);
                    allVariables[i.first].push_back(j.second);
                }
            }
        }
        for(auto& entry : modbusIpProcessMap) {
            entry.second->start();
        }
        for(auto& entry : modbusRtuProcessMap) {
            entry.second->start();
        }
		TcpTalker tcpTalker(56789);
		tcpTalker.set_target(variableTree);
		tcpTalker.start();
		while(1) {
			this_thread::sleep_for(1s);
		}
    }
};