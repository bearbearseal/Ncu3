#include "ModbusRtuProcess.h"
#include "ModbusRTU.h"
#include <iostream>

using namespace std;

ModbusRtuProcess::ModbusRtuProcess(shared_ptr<SyncedSerialPort> _serialPort, uint8_t slaveAddress, uint16_t maxRegisterPerMessage, uint16_t maxCoilPerMessage, std::chrono::milliseconds timeout, bool smallEndian) {
	myShadow = make_shared<Shadow>(*this);
	serialPort = _serialPort;
	config.slaveAddress = slaveAddress;
	config.maxRegisterPerMessage = maxRegisterPerMessage;
	config.maxCoilPerMessage = maxCoilPerMessage;
	config.timeout = timeout;
	config.smallEndian = smallEndian;
	keepRunning = false;
}

ModbusRtuProcess::~ModbusRtuProcess() {
    this->stop();
    std::weak_ptr<Shadow> weak = myShadow;
    myShadow.reset();
    while(weak.lock() != nullptr) {
        this_thread::yield();
    }
}

void ModbusRtuProcess::start() {
    if(theThread != nullptr) {
        return;         
    }
	build_query();
	clean_build_data();
    keepRunning = true;
    theThread = make_unique<thread>(thread_process, this);
}

void ModbusRtuProcess::stop() {
    keepRunning = false;
    if(theThread != nullptr) {
        theThread->join();
        theThread = nullptr;
    }
}

shared_ptr<ModbusRtuProcess::CoilStatusVariable> ModbusRtuProcess::create_coil_status_variable(uint16_t coilAddress) {
    lock_guard<mutex> lock(coilVariableMutex);
    auto i = address2CoilVariableMap.find(coilAddress);
    if(i != address2CoilVariableMap.end()) {
        return i->second;
    }
    auto retVal = make_shared<ModbusRtuProcess::CoilStatusVariable>(myShadow, coilAddress);
    address2CoilVariableMap.insert({coilAddress, retVal});
    return retVal;
}

shared_ptr<ModbusRtuProcess::HoldingRegisterVariable> ModbusRtuProcess::create_holding_register_variable(uint16_t registerAddress, ModbusRegisterValue::DataType type) {
	uint16_t count = ModbusRegisterValue::get_register_count(type);
	shared_ptr<HoldingRegisterVariable> retVal = make_shared<HoldingRegisterVariable>(myShadow, registerAddress, type, config.smallEndian);
	HoldingRegisterData& data = address2HoldingRegisterVariableMap[registerAddress];
	if (data.count < count)
	{
		data.count = count;
	}
	data.variables.push_back(retVal);
	return retVal;;

}

void ModbusRtuProcess::add_write_holding_register(uint16_t registerAddress, std::vector<RegisterValue>& registerValues) {
	lock_guard<mutex> aLock(writeHoldingRegisterData.theMutex);
	auto i = writeHoldingRegisterData.address2ValueWriteMap.find(registerAddress);
	if(i == writeHoldingRegisterData.address2ValueWriteMap.end()) {
		writeHoldingRegisterData.writeOrder.push_back(registerAddress);
	}
	writeHoldingRegisterData.address2ValueWriteMap[registerAddress] = move(registerValues);
}

void ModbusRtuProcess::write_holding_register() {
	unordered_map<uint32_t, vector<RegisterValue>> address2ValueWriteMap;
	vector<uint32_t> writeOrder;
	{
		lock_guard<mutex> aLock(writeHoldingRegisterData.theMutex);
		address2ValueWriteMap = move(writeHoldingRegisterData.address2ValueWriteMap);
		writeOrder = move(writeHoldingRegisterData.writeOrder);
	}
	for (unsigned i = 0; i < writeOrder.size(); ++i) {
		vector<RegisterValue>& values = address2ValueWriteMap[writeOrder[i]];	//make sure values size is at least 1
		uint16_t registerAddress = writeOrder[i];
		pair<string, uint16_t> query;	//query string and expected reply length
		if (values.size() > 1) {
			query = ModbusRTU::create_preset_multiple_registers(config.slaveAddress, registerAddress, values);
		}
		else {
			query = ModbusRTU::create_preset_single_register(config.slaveAddress, registerAddress, values[0]);
		}
		for (unsigned j=0; j<2; ++j) {	//try at most 2 times
			string reply = serialPort->write_then_read(query.first, query.second, config.timeout);
			if (values.size() == 1) {	//Preset single register
				if(reply.size() >= query.second) {
					auto replyData = ModbusRTU::decode_reply(reply);
					if (replyData.functionCode == ModbusRTU::PRESET_SINGLE_REGISTER_CODE) {
						auto presetSingleRegister = replyData.get_preset_single_register();	//returns address and value pair
						if(presetSingleRegister.first == registerAddress && presetSingleRegister.second == values[0]) {	//Both values same, write was succesful
							break;
						}
					}
				}
			}
			else {	//Preset multiple register
				if(reply.size() >= query.second) {
					auto replyData = ModbusRTU::decode_reply(reply); 
					if (replyData.functionCode == ModbusRTU::PRESET_MULTIPLE_REGISTERS_CODE)
					{
						auto presetMultipleRegister = replyData.get_preset_multiple_registers();	//returns address and register count
						if(presetMultipleRegister.first == registerAddress && presetMultipleRegister.second == values.size()) {	//wrtie successfull
							break;
						}
					}
				}
			}
		}
	}
}

void ModbusRtuProcess::add_force_coil_status(uint16_t coilAddress, bool newValue) {
	lock_guard<mutex> aLock(forceCoilData.theMutex);
	forceCoilData.address2ValueWriteMap[coilAddress] = newValue;
}

void ModbusRtuProcess::force_coil_status() {
	map<uint16_t, bool> theMap;
	{
		lock_guard<mutex> aLock(forceCoilData.theMutex);
		theMap = move(forceCoilData.address2ValueWriteMap);
	}
	for (auto i = theMap.begin(); i != theMap.end(); ++i) {
		auto query = ModbusRTU::create_force_single_coil(config.slaveAddress, i->first, i->second);	//query string and expected reply length
		for (unsigned j = 0; j < 2; ++j) {
			string reply = serialPort->write_then_read(query.first, query.second, config.timeout);
			if(reply.size() >= query.second) {
				auto replyData = ModbusRTU::decode_reply(reply);
				if (replyData.functionCode == ModbusRTU::FORCE_SINGLE_COIL_CODE) {
					auto forceSingleCoil = replyData.get_force_single_coil();
					if(forceSingleCoil.first == i->first && forceSingleCoil.second == i->second) {	//write successfull
						break;
					}
				}

			}
		}
	}
}

void ModbusRtuProcess::build_query() {
	//Coil Status
	{
		struct CoilStatusQueryData {
			uint16_t firstAddress;
			uint16_t coilCount = 0;
			vector<shared_ptr<CoilStatusVariable>> variables;
		} entry;
		for(auto i = address2CoilVariableMap.begin(); i != address2CoilVariableMap.end(); ++i) {
			if(!bool(entry.coilCount)) {
				entry.firstAddress = i->first;
				entry.coilCount = 1;
				entry.variables.push_back(i->second);
				continue;
			}
			if(i->first - (entry.firstAddress + entry.coilCount) < 8) {
				if(config.maxCoilPerMessage > (i->first - entry.firstAddress)) {
					entry.coilCount = i->first - entry.firstAddress + 1;
					entry.variables.push_back(i->second);
					continue;
				}
			}
			auto result = ModbusRTU::create_read_coil_status(config.slaveAddress, entry.firstAddress, entry.coilCount);
			CoilStatusQuery element;
			element.query = result.first;
			element.replyLength = result.second;
			element.variables = move(entry.variables);
			element.startAddress = entry.firstAddress;
			coilStatusQueryList.push_back(element);
			entry.firstAddress = i->first;
			entry.coilCount = 1;
			entry.variables.push_back(i->second);
		}
		if (bool(entry.coilCount)) {
			auto result = ModbusRTU::create_read_coil_status(config.slaveAddress, entry.firstAddress, entry.coilCount);
			CoilStatusQuery element;
			element.query = result.first;
			element.replyLength = result.second;
			element.variables = move(entry.variables);
			element.startAddress = entry.firstAddress;
			coilStatusQueryList.push_back(element);
		}
	}
	//Holding Register
	{
		struct HoldingRegisterQueryData {
			uint16_t registerAddress;
			uint16_t registerCount = 0;
			vector<shared_ptr<HoldingRegisterVariable>> variables;
		} entry;
		for(auto i = address2HoldingRegisterVariableMap.begin(); i != address2HoldingRegisterVariableMap.end(); ++i) {
			if(!bool(entry.registerCount)) {
				entry.registerAddress = i->first;
				entry.registerCount = i->second.count;
				entry.variables = i->second.variables;
				continue;
			}
			if(entry.registerAddress + entry.registerCount == i->first) {
				if(config.maxRegisterPerMessage >= entry.registerCount + i->second.count) {
					entry.registerCount += i->second.count;
					entry.variables.insert(entry.variables.end(), i->second.variables.begin(), i->second.variables.end());
					continue;
				}
			}
			auto result = ModbusRTU::create_read_holding_register(config.slaveAddress, entry.registerAddress, entry.registerCount);
			HoldingRegisterQuery element;
			element.query = result.first;
			element.replyLength = result.second;
			element.variables = move(entry.variables);
			element.startAddress = entry.registerAddress;
			holdingRegisterQueryList.push_back(element);
			entry.registerAddress = i->first;
			entry.registerCount = i->second.count;
			entry.variables = i->second.variables;
		}
		if(bool(entry.registerCount)) {
			auto result = ModbusRTU::create_read_holding_register(config.slaveAddress, entry.registerAddress, entry.registerCount);
			HoldingRegisterQuery element;
			element.query = result.first;
			element.replyLength = result.second;
			element.variables = move(entry.variables);
			element.startAddress = entry.registerAddress;
			holdingRegisterQueryList.push_back(element);
		}
	}
}

void ModbusRtuProcess::clean_build_data() {
	address2HoldingRegisterVariableMap.clear();
	address2CoilVariableMap.clear();
}

void ModbusRtuProcess::thread_process(ModbusRtuProcess* theProcess) {
	printf("Modbus Rtu started.\n");
	while (theProcess->keepRunning) {
		//do write
		for(HoldingRegisterQuery& singleQuery : theProcess->holdingRegisterQueryList) {
			//Make sure write is handled frequently {
			theProcess->write_holding_register();
			theProcess->force_coil_status();
			//}
			string reply = theProcess->serialPort->write_then_read(singleQuery.query, singleQuery.replyLength, theProcess->config.timeout);
			if(reply.size() >= singleQuery.replyLength) {
				auto replyData = ModbusRTU::decode_reply(reply);
				if (replyData.functionCode == ModbusRTU::READ_HOLDING_REGISTER_CODE) {
					vector<RegisterValue> result = replyData.get_holding_register();
					for(auto element : singleQuery.variables) {
						element->update_value_from_source(singleQuery.startAddress, result);
					}
				}
			}
		}
		for(CoilStatusQuery& singleQuery : theProcess->coilStatusQueryList) {
			//Make sure write is handled frequently {}
			theProcess->write_holding_register();
			theProcess->force_coil_status();
			//}
			string reply = theProcess->serialPort->write_then_read(singleQuery.query, singleQuery.replyLength, theProcess->config.timeout);
			if(reply.size() >= singleQuery.replyLength) {
				auto replyData = ModbusRTU::decode_reply(reply);
				if(replyData.functionCode == ModbusRTU::READ_COIL_CODE) {
					vector<bool> result = replyData.get_coils();
					for(auto element : singleQuery.variables) {
						element->update_value_from_source(singleQuery.startAddress, result);
					}
				}
			}
		}
		this_thread::sleep_for(2000ms);
	}
}

//Coil Status Variable {
ModbusRtuProcess::CoilStatusVariable::CoilStatusVariable(std::shared_ptr<Shadow> _master, uint16_t _coilAddress) : master(_master), coilAddress(_coilAddress){

}

ModbusRtuProcess::CoilStatusVariable::~CoilStatusVariable() {

}

void ModbusRtuProcess::CoilStatusVariable::update_value_from_source(uint16_t firstAddress, const std::vector<bool>& values) {
	size_t index = coilAddress - firstAddress;
	if (index >= values.size())
	{
		return;
	}
	this->update_value_to_cache(values[index]);
}
//}

//Holding Register Variable {
ModbusRtuProcess::HoldingRegisterVariable::HoldingRegisterVariable(std::shared_ptr<Shadow> _master, uint16_t _firstAddress, ModbusRegisterValue::DataType _type, bool _isSmallEndian) {
	master = _master;
	firstAddress = _firstAddress;
	type = _type;
	isSmallEndian = _isSmallEndian;
}

ModbusRtuProcess::HoldingRegisterVariable::~HoldingRegisterVariable() {

}

void ModbusRtuProcess::HoldingRegisterVariable::update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue>& values) {
	size_t index = firstAddress - _registerAddress;
	if (index >= values.size())
	{
		return;
	}
	ModbusRegisterValue modbusValue(type, isSmallEndian);
	this->update_value_to_cache(modbusValue.get_value());
}
//}
