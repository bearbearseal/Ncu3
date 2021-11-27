#include "VariableTree/VariableTree.h"
#include "VariableTree/TcpTalker.h"
//#include "Builder/Builder.h"
#include "Storage/ConfigStorage.h"
#include "Integrator/ChannelManager.h"
//#include "Integrator/EquipmentManager.h"
#include "Integrator/SerialPortManager.h"
#include "InOutOperation/OpStorage.h"
#include "Schedule/ScheduleManager.h"
#include "Alarm/AlarmProcessor.h"
#include "Alarm/AlarmVerifyStorePost.h"
#include "Alarm/AlarmTalker.h"

#include <thread>

using namespace std;

namespace Deploy {
    /*
    void run_real_time_value() {
        shared_ptr<VariableTree> variableTree = make_shared<VariableTree>();
        unordered_map<size_t, unique_ptr<ModbusIpProcess>> modbusIpProcessMap;
        unordered_map<size_t, unique_ptr<ModbusRtuProcess>> modbusRtuProcessMap;
        unordered_map<size_t, vector<shared_ptr<Variable>>> allVariables;
		shared_ptr<AlarmHandler> alarmHandler = make_shared<AlarmHandler>("/var/sqlite/NcuAlarm.db", "127.0.0.1", 33333);
        //Create alarm detector
        unordered_map<HashKey::DualKey, shared_ptr<AlarmDetector>, HashKey::DualKeyHash> key2Detector;
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
            auto alarmLogicMap = aBuilder.get_logic();
            auto alarmLogicPair = aBuilder.get_logic_pair();
            printf("Total logic: %lu\n", alarmLogicMap.size());
            printf("Total logic pair: %lu\n", alarmLogicPair.size());
            for(auto& i : alarmLogicPair) {
                key2Detector.emplace(i.first, make_shared<AlarmDetector>(i.first.get_first(), i.first.get_second()));
                for(auto& j : i.second) {
                    auto theLogic = alarmLogicMap.find(j.logicId);
                    if(theLogic != alarmLogicMap.end()) {
                        key2Detector[i.first]->add_logic(j.priority, theLogic->second);
                    }
                }
            }
            printf("No fault.\n");
            for(auto& i : key2Detector) {
                //Set listener for alarm detector
                i.second->set_alarm_listener(alarmHandler);
                //Add detector to each point
                auto equipmentBranch = variableTree->get_child(i.first.get_first());
                if(equipmentBranch != nullptr) {
                    auto propertyLeaf = equipmentBranch->get_child(i.first.get_second());
                    if(propertyLeaf != nullptr) {
                        printf("Applying alarm logic to %s %s\n", i.first.get_first().to_string().c_str(), i.first.get_second().to_string().c_str());
                        propertyLeaf->add_value_change_listener(i.second);
                    }
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
		alarmHandler->start();
		while(1) {
			this_thread::sleep_for(1s);
		}
    }
    */
/*
    void run_equipment() {
        ConfigStorage configData("/var/sqlite/NcuConfig.db");
        SerialPortManager serialPortManager(configData);
        ChannelManager channelManager(configData, serialPortManager);
        OpStorage opStorage("/var/sqlite/NcuConfig.db", "/var/InOutOp");
        EquipmentManager equipmentManager(configData, channelManager, opStorage);

        shared_ptr<VariableTree> root = make_shared<VariableTree>();
        equipmentManager.attach_equipments(root, true, true);
		ScheduleManager scheduleManager(configData, root);
        scheduleManager.start();
        TcpTalker tcpTalker(10520);
        tcpTalker.set_target(root);
        tcpTalker.start();
        channelManager.start();
        while(1) {
            this_thread::sleep_for(1s);
        }
    }
*/
/*
    void run_equipment() {
        ConfigStorage configData("/var/sqlite/NcuConfig.db");
        SerialPortManager serialPortManager(configData);
        ChannelManager channelManager(configData, serialPortManager);
        OpStorage opStorage("/var/sqlite/NcuConfig.db", "/var/InOutOp");
        EquipmentManager equipmentManager(configData, channelManager, opStorage);

        shared_ptr<VariableTree> root = make_shared<VariableTree>();
        equipmentManager.attach_equipments(root, true, true);

		ScheduleManager scheduleManager(configData, root);
        scheduleManager.start();
        TcpTalker tcpTalker(10520);
        tcpTalker.set_target(root);
        tcpTalker.start();
        channelManager.start();
        while(1) {
            this_thread::sleep_for(1s);
        }
    } 
    */
   void run_equipment_alarm() {
        ConfigStorage configData("/var/sqlite/NcuConfig.db");
        SerialPortManager serialPortManager(configData);
        OpStorage opStorage("/var/sqlite/NcuConfig.db", "/var/InOutOp");
        ChannelManager channelManager(configData, serialPortManager, opStorage);

        shared_ptr<VariableTree> root = make_shared<VariableTree>();
        channelManager.attach_to_tree(root);

        shared_ptr<AlarmVerifyStorePost> alarmVerifyStorePost = make_shared<AlarmVerifyStorePost>("/var/sqlite/NcuAlarm.db");
        AlarmTalker alarmTalker(10521, alarmVerifyStorePost);
        AlarmProcessor alamProcessor(configData, root, alarmVerifyStorePost);
        alarmTalker.start();

        TcpTalker tcpTalker(10520);
        tcpTalker.set_target(root);
        tcpTalker.start();
        channelManager.start();
        while(1) {
            this_thread::sleep_for(1s);
        }

   }
};