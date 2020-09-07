#include "ModbusProcess.h"
#include "ModbusRegisterValue.h"
#include "PrintModbus.h"
#include "ModbusRTU.h"
#include <thread>
#include <chrono>
#include <tuple>

using namespace std;

uint32_t blend_slave_register_address(uint8_t slaveAddress, uint16_t registerAddress)
{
	uint32_t retVal = slaveAddress;
	retVal <<= 16;
	retVal |= registerAddress;
	return retVal;
}

pair<uint8_t, uint16_t> get_slave_register_address(uint32_t slaveRegisterAddress)
{
	uint8_t slaveAddress = (uint8_t)(slaveRegisterAddress >> 16) & 0xff;
	uint16_t registerAddress = (uint16_t)(slaveRegisterAddress);
	return { slaveAddress, registerAddress };
}

ModbusProcess::ModbusProcess(SerialPort& _serialPort):serialPort(_serialPort)
{

}

ModbusProcess::~ModbusProcess()
{

}

shared_ptr<ModbusProcess::CoilStatusVariable> ModbusProcess::get_coil_status_variable(uint8_t slaveAddress, uint16_t coilAddress)
{
	if (!slaveDataMap[slaveAddress].coilStatus.variables.count(coilAddress))
	{
		slaveDataMap[slaveAddress].coilStatus.variables[coilAddress] = make_shared<ModbusProcess::CoilStatusVariable>(*this, slaveAddress, coilAddress);
	}
	return slaveDataMap[slaveAddress].coilStatus.variables[coilAddress];
}

shared_ptr<ModbusProcess::HoldingRegisterVariable> ModbusProcess::get_holding_register_variable(
	uint8_t slaveAddress,
	uint16_t registerAddress,
	ModbusRegisterValue::DataType type,
	bool smallEndian)
{
	uint16_t count = ModbusRegisterValue::get_register_count(type);
	shared_ptr<HoldingRegisterVariable> retVal = make_shared<HoldingRegisterVariable>(*this, slaveAddress, registerAddress, type, smallEndian);
	HoldingRegisterData& data = slaveDataMap[slaveAddress].holdingRegister.address2VariableReadMap[registerAddress];
	if (data.count < count)
	{
		data.count = count;
	}
	data.variables.push_back(retVal);
	return retVal;;
}

void ModbusProcess::start()
{
	build_query();
	clean_build_data();
	thread* aProcess = new thread(thread_process, this);
}

bool ModbusProcess::set_slave(uint8_t address, uint16_t maxRegister, uint16_t maxCoilPerMessage, bool smallEndian)
{
	/*
	if (slaveDataMap.count(address))
	{
		return false;
	}
	*/
	slaveDataMap[address].coilStatus.maxCoilPerQuery = maxCoilPerMessage;
	slaveDataMap[address].holdingRegister.maxPerQuery = maxRegister;
	return true;
}

void ModbusProcess::add_force_coil_status(uint8_t slaveAddress, uint16_t coilAddress, bool newValue)
{
	lock_guard<mutex> aLock(forceCoilData.lock);
	std::map<uint16_t, bool>& entry = forceCoilData.slave2CoilAddress2ValueWriteMap[slaveAddress];
	entry[coilAddress] = newValue;
}

void ModbusProcess::add_write_holding_register(uint8_t slaveAddress, uint16_t registerAddress, vector<RegisterValue>&& registerValues)
{
	uint32_t slaveRegisterAddress = blend_slave_register_address(slaveAddress, registerAddress);
	lock_guard<mutex> aLock(writeHoldingRegisterData.lock);
	if (!writeHoldingRegisterData.slaveRegister2ValueWriteMap.count(slaveRegisterAddress))
	{
		writeHoldingRegisterData.writeOrder.push_back(slaveRegisterAddress);
	}
	writeHoldingRegisterData.slaveRegister2ValueWriteMap[slaveRegisterAddress] = registerValues;
}

tuple<bool, uint16_t, bool> get_force_single_coil_reply(SerialPort* serialPort, uint16_t expectedLength)
{
	chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
	string reply;
	do {
		this_thread::sleep_for(20ms);
		reply += serialPort->read();
		if (reply.size() >= expectedLength)
		{
			auto replyData = ModbusRTU::decode_reply(reply);
			if (replyData.functionCode == ModbusRTU::FORCE_SINGLE_COIL_CODE)
			{
				auto forceSingleCoil = replyData.get_force_single_coil();
				return { true, forceSingleCoil.first, forceSingleCoil.second };
				break;
			}
			reply.clear();
		}
	} while (chrono::steady_clock::now() - beginTime < 2s);
	return { false, 0, true };
}

tuple<bool, uint16_t, RegisterValue> get_preset_single_register_reply(SerialPort* serialPort, uint16_t expectedLength)
{
	chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
	string reply;
	do {
		this_thread::sleep_for(20ms);
		reply += serialPort->read();
		if (reply.size() >= expectedLength)
		{
			auto replyData = ModbusRTU::decode_reply(reply);
			if (replyData.functionCode == ModbusRTU::PRESET_SINGLE_REGISTER_CODE)
			{
				auto presetSingleRegister = replyData.get_preset_single_register();
				return {true, presetSingleRegister.first, presetSingleRegister.second };
				break;
			}
			reply.clear();
		}
	} while (chrono::steady_clock::now() - beginTime < 2s);
	return {false, 0, RegisterValue()};
}

tuple<bool, uint16_t, uint16_t> get_preset_multiple_holding_register_reply(SerialPort* serialPort, uint16_t expectedLength)
{
	chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
	string reply;
	do {
		this_thread::sleep_for(20ms);
		reply += serialPort->read();
		if (reply.size() >= expectedLength)
		{
			//printf("Read:");
			//PrintModbus::print_value_in_hex(reply);
			//printf("B4 decode.\n");
			auto replyData = ModbusRTU::decode_reply(reply);
			//printf("After decode.\n");
			if (replyData.functionCode == ModbusRTU::PRESET_MULTIPLE_REGISTERS_CODE)
			{
				//printf("Reply Data after %llu ms:", chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - beginTime).count());
				auto presetMultipleRegister = replyData.get_preset_multiple_registers();
				return { true, presetMultipleRegister.first, presetMultipleRegister.second };
				//printf("\n");
				break;
			}
			reply.clear();
		}
	} while (chrono::steady_clock::now() - beginTime < 2s);
	return { false, 0, 0 };
}

void ModbusProcess::force_coil_status()
{
	unordered_map<uint8_t, map<uint16_t, bool>> theMap;
	{
		lock_guard<mutex> aLock(forceCoilData.lock);
		theMap = move(forceCoilData.slave2CoilAddress2ValueWriteMap);
	}
	for (auto i = theMap.begin(); i != theMap.end(); ++i)
	{
		for (auto j = i->second.begin(); j != i->second.end(); ++j)
		{
			//printf("Forcing coil.\n");
			auto query = ModbusRTU::create_force_single_coil(i->first, j->first, j->second);
			for (unsigned k = 0; k < 2; ++k)
			{
				serialPort.write(query.first);
				auto result = get_force_single_coil_reply(&serialPort, query.second);
				if (get<0>(result) && get<1>(result) == j->first && get<2>(result) == j->second)
				{
					break;
				}
			}
		}
	}
}

void ModbusProcess::write_holding_register()
{
	unordered_map<uint32_t, vector<RegisterValue>> slaveRegister2ValueWriteMap;
	vector<uint32_t> writeOrder;
	{
		lock_guard<mutex> aLock(writeHoldingRegisterData.lock);
		slaveRegister2ValueWriteMap = move(writeHoldingRegisterData.slaveRegister2ValueWriteMap);
		writeOrder = move(writeHoldingRegisterData.writeOrder);
	}
	for (unsigned i = 0; i < writeOrder.size(); ++i)
	{
		vector<RegisterValue>& values = slaveRegister2ValueWriteMap[writeOrder[i]];	//make sure values size is at least 1
		auto thePair = get_slave_register_address(writeOrder[i]);
		uint8_t slaveAddress = thePair.first;
		uint16_t registerAddress = thePair.second;
		pair<string, uint16_t> query;
		if (values.size() > 1)
		{
			query = ModbusRTU::create_preset_multiple_registers(slaveAddress, registerAddress, values);
		}
		else
		{
			query = ModbusRTU::create_preset_single_register(slaveAddress, registerAddress, values[0]);
		}
		for (unsigned j=0; j<2; ++j)	//try at most 2 times
		{
			//printf("Writing holding register.\n");
			//printf("Serial port writing:\n");
			//PrintModbus::print_value_in_hex(query.first);
			serialPort.write(query.first);
			if (values.size() == 1)
			{
				tuple<bool, uint16_t, RegisterValue> result = get_preset_single_register_reply(&serialPort, query.second);
				if (get<0>(result))
				{
					if (get<1>(result) == registerAddress && get<2>(result)==values[0])
					{
						break;
					}
				}
			}
			else
			{
				tuple<bool, uint16_t, uint16_t> result = get_preset_multiple_holding_register_reply(&serialPort, query.second);
				if (get<0>(result))
				{
					if (get<1>(result) == registerAddress && get<2>(result)==values.size())
					{
						break;
					}
				}
			}
		}
	}
}

void ModbusProcess::build_query()
{
	//Build the vector
	for (unordered_map<uint8_t, SlaveData>::iterator i = slaveDataMap.begin(); i != slaveDataMap.end(); ++i)
	{
		//Coil Status
		{
			struct CoilStatusQueryData
			{
				uint8_t slaveAddress;
				uint16_t firstAddress;
				uint16_t coilCount;
				vector<shared_ptr<CoilStatusVariable>> variables;
			}entry;
			bool isEmpty = true;
			map<uint16_t, shared_ptr<CoilStatusVariable>>& subMap = i->second.coilStatus.variables;
			printf("Coil count: %u\n", subMap.size());
			for (map<uint16_t, shared_ptr<CoilStatusVariable>>::iterator j = subMap.begin(); j != subMap.end(); ++j)
			{
				if (isEmpty)
				{
					isEmpty = false;
					entry.slaveAddress = i->first;
					entry.firstAddress = j->first;
					entry.coilCount = 1;
					entry.variables.push_back(j->second);
					continue;
				}
				else if(j->first - (entry.firstAddress + entry.coilCount) < 8)
				{
					if (i->second.coilStatus.maxCoilPerQuery > (j->first - entry.firstAddress))
					{
						printf("Another coil pushing into same query, coilAddress %u coilCount %u newAddress %u\n",
							entry.firstAddress, j->first - entry.firstAddress, j->first);
						entry.coilCount = j->first - entry.firstAddress + 1;
						entry.variables.push_back(j->second);
						continue;
					}
				}
				//Cannot append
				//printf("Creating entry\n\tSlave Address: %u\n\tCoilAddress: %u\n\tCoilCount: %u\n", entry.slaveAddress, entry.firstAddress, entry.coilCount);
				auto result = ModbusRTU::create_read_coil_status(entry.slaveAddress, entry.firstAddress, entry.coilCount);
				CoilStatusQuery element;
				element.query = result.first;
				element.replyLength = result.second;
				element.variables = move(entry.variables);
				element.startAddress = entry.firstAddress;
				coilStatusQueryList.push_back(element);
				entry.slaveAddress = i->first;
				entry.firstAddress = j->first;
				entry.coilCount = 1;
				entry.variables.clear();
				entry.variables.push_back(j->second);
			}
			if (!isEmpty)
			{
				//printf("Creating entry\n\tSlave Address: %u\n\tCoilAddress: %u\n\tCoilCount: %u\n", entry.slaveAddress, entry.firstAddress, entry.coilCount);
				auto result = ModbusRTU::create_read_coil_status(entry.slaveAddress, entry.firstAddress, entry.coilCount);
				CoilStatusQuery element;
				element.query = result.first;
				element.replyLength = result.second;
				element.variables = move(entry.variables);
				element.startAddress = entry.firstAddress;
				coilStatusQueryList.push_back(element);
			}
		}
		//Holding register
		{
			struct HoldingRegisterQueryData
			{
				uint8_t slaveAddress;
				uint16_t registerAddress;
				uint16_t registerCount;
				vector<shared_ptr<HoldingRegisterVariable>> variables;
			}entry;
			bool isEmpty = true;
			map<uint16_t, HoldingRegisterData>& subMap = i->second.holdingRegister.address2VariableReadMap;
			for (map<uint16_t, HoldingRegisterData>::iterator j = subMap.begin(); j != subMap.end(); ++j)
			{
				if (isEmpty)
				{
					isEmpty = false;
					entry.slaveAddress = i->first;
					entry.registerAddress = j->first;
					entry.registerCount = j->second.count;
					entry.variables = j->second.variables;
					continue;
				}
				else if (entry.registerAddress + entry.registerCount == j->first)
				{
					if (i->second.holdingRegister.maxPerQuery >= entry.registerCount + j->second.count)
					{
						entry.registerCount += j->second.count;
						entry.variables.insert(entry.variables.end(), j->second.variables.begin(), j->second.variables.end());
						continue;
					}
				}
				//Cannot append
				//printf("Creating entry\n\tSlave Address: %u\n\tRegisterAddress: %u\n\tRegister Count: %u\n", entry.slaveAddress, entry.registerAddress, entry.registerCount);
				auto result = ModbusRTU::create_read_holding_register(entry.slaveAddress, entry.registerAddress, entry.registerCount);
				HoldingRegisterQuery element;
				element.query = result.first;
				element.replyLength = result.second;
				element.variables = move(entry.variables);
				element.startAddress = entry.registerAddress;
				holdingRegisterQueryList.push_back(element);
				entry.slaveAddress = i->first;
				entry.registerAddress = j->first;
				entry.registerCount = j->second.count;
				entry.variables = j->second.variables;
			}
			if (!isEmpty)
			{
				//printf("Creating entry\n\tSlave Address: %u\n\tRegisterAddress: %u\n\tRegister Count: %u\n", entry.slaveAddress, entry.registerAddress, entry.registerCount);
				auto result = ModbusRTU::create_read_holding_register(entry.slaveAddress, entry.registerAddress, entry.registerCount);
				HoldingRegisterQuery element;
				element.query = result.first;
				element.replyLength = result.second;
				element.variables = move(entry.variables);
				element.startAddress = entry.registerAddress;
				holdingRegisterQueryList.push_back(element);
			}
		}
	}
}

void ModbusProcess::clean_build_data()
{
	slaveDataMap.clear();
}

ModbusProcess::CoilStatusQuery* ModbusProcess::get_next_coil_status_query()
{
	if (coilStatusQueryIndex >= coilStatusQueryList.size())
	{
		return nullptr;
	}
	CoilStatusQuery* retVal = &coilStatusQueryList[coilStatusQueryIndex];
	++coilStatusQueryIndex;
	return retVal;
}

ModbusProcess::HoldingRegisterQuery* ModbusProcess::get_next_holding_register_query()
{
	if (holdingRegisterQueryIndex >= holdingRegisterQueryList.size())
	{
		return nullptr;
	}
	HoldingRegisterQuery* retVal = &holdingRegisterQueryList[holdingRegisterQueryIndex];
	++holdingRegisterQueryIndex;
	return retVal;
}

vector<bool> get_coil_status_reply(SerialPort* serialPort, uint16_t expectedLength)
{
	vector<bool> retVal;
	chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
	string reply;
	do {
		this_thread::sleep_for(20ms);
		reply += serialPort->read();
		if (reply.size() >= expectedLength)
		{
			auto replyData = ModbusRTU::decode_reply(reply);
			if (replyData.functionCode == ModbusRTU::READ_COIL_CODE)
			{
				retVal = replyData.get_coils();
				break;
			}
			reply.clear();
		}
	} while (chrono::steady_clock::now() - beginTime < 2s);
	return retVal;
}

vector<RegisterValue> get_holding_register_reply(SerialPort* serialPort, uint16_t expectedLength)
{
	vector<RegisterValue> retVal;
	chrono::time_point<chrono::steady_clock> beginTime = chrono::steady_clock::now();
	string reply;
	do {
		this_thread::sleep_for(20ms);
		reply += serialPort->read();
		if (reply.size() >= expectedLength)
		{
			//printf("Read:");
			//PrintModbus::print_value_in_hex(reply);
			auto replyData = ModbusRTU::decode_reply(reply);
			if (replyData.functionCode == ModbusRTU::READ_HOLDING_REGISTER_CODE)
			{
				//printf("Reply Data after %llu ms:", chrono::duration_cast<std::chrono::milliseconds>(chrono::steady_clock::now() - beginTime).count());
				retVal = replyData.get_holding_register();
				/*
				auto holdingRegister = replyData.get_holding_register();
				for (unsigned j = 0; j < holdingRegister.size(); ++j)
				{
					//printf("[H:%X][L:%X]", holdingRegister[j].high, holdingRegister[j].low);
					retVal.push_back(holdingRegister[j]);
				}
				//printf("\n");
				//this_thread::sleep_for(10ms);
				*/
				break;
			}
			reply.clear();
		}
	} while (chrono::steady_clock::now() - beginTime < 2s);
	return retVal;
}

void ModbusProcess::thread_process(ModbusProcess* theProcess)
{
	while (1)
	{
		//do write
		theProcess->write_holding_register();
		theProcess->force_coil_status();
		switch (theProcess->machineState)
		{
		case 0:	//Holding Register
		{
			HoldingRegisterQuery* holdingRegisterQuery = theProcess->get_next_holding_register_query();
			if (holdingRegisterQuery != nullptr)
			{
				//do query
				theProcess->serialPort.write(holdingRegisterQuery->query);
				vector<RegisterValue> result = get_holding_register_reply(&theProcess->serialPort, holdingRegisterQuery->replyLength);
				//pass values to subscribers
				for (unsigned i = 0; i < holdingRegisterQuery->variables.size(); ++i)
				{
					holdingRegisterQuery->variables[i]->update_value_from_source(holdingRegisterQuery->startAddress, result);
				}
			}
			else
			{
				theProcess->machineState = 1;
			}
			break;
		}
		case 1:	//Coil Status
		{
			CoilStatusQuery* coilStatusQuery = theProcess->get_next_coil_status_query();
			if (coilStatusQuery != nullptr)
			{
				theProcess->serialPort.write(coilStatusQuery->query);
				vector<bool> result = get_coil_status_reply(&theProcess->serialPort, coilStatusQuery->replyLength);
				for (unsigned i=0; i<coilStatusQuery->variables.size(); ++i)
				{
					coilStatusQuery->variables[i]->update_value_from_source(coilStatusQuery->startAddress, result);
				}
			}
			else
			{
				theProcess->machineState = 10;
			}
			break;
		}
		case 10:
			theProcess->reset_holding_register_query();
			theProcess->reset_coil_status_query();
			theProcess->machineState = 0;
			break;
		}
		this_thread::sleep_for(200ms);
	}
}

ModbusProcess::HoldingRegisterVariable::HoldingRegisterVariable(
		ModbusProcess& _master,
		uint8_t _slaveAddress,
		uint16_t _firstAddress,
		ModbusRegisterValue::DataType _type,
		bool _smallEndian)
: master(_master)
{
	slaveAddress = _slaveAddress;
	firstAddress = _firstAddress;
	type = _type;
	smallEndian = _smallEndian;
}

ModbusProcess::HoldingRegisterVariable::~HoldingRegisterVariable()
{

}

void ModbusProcess::HoldingRegisterVariable::set_value(const Value& newValue)
{
	ModbusRegisterValue setValue(type, smallEndian);
	std::vector<RegisterValue> converted = setValue.convert_to_register_value(newValue);
	master.add_write_holding_register(slaveAddress, firstAddress, move(converted));
}

Value ModbusProcess::HoldingRegisterVariable::get_value() const
{
	lock_guard<mutex> locker(valueLock);
	Value retVal = value;
	return retVal;;
}

pair<Value, chrono::time_point<chrono::system_clock>> ModbusProcess::HoldingRegisterVariable::get_value_with_time() const
{
	lock_guard<mutex> locker(valueLock);
	Value retVal = value;
	chrono::time_point<chrono::system_clock> retVal2 = timePoint;
	return {retVal, retVal2};
}

void ModbusProcess::CoilStatusVariable::update_value_from_source(uint16_t firstAddress, const std::vector<bool>& values)
{
	size_t index = coilAddress - firstAddress;
	if (index >= values.size())
	{
		return;
	}
	bool update = false;
	{
		lock_guard<mutex> locker(valueLock);
		if(value != values[index])
		{
			update = true;
			value = values[index];
		}
		timePoint = chrono::system_clock::now();
	}
	if(update)
	{
		this->update_value_to_subscribers(value, timePoint);
	}
}

void ModbusProcess::HoldingRegisterVariable::update_value_from_source(uint16_t _registerAddress, const vector<RegisterValue>& values)
{
	size_t index = firstAddress - _registerAddress;
	if (index >= values.size())
	{
		return;
	}
	ModbusRegisterValue modbusValue(type, smallEndian);

	bool update = false;
	modbusValue.set_register_value(values, index);
	{
		lock_guard<mutex> locker(valueLock);
		if(value != modbusValue.get_value())
		{
			//printf("%llu != %llu\n", value.get_int(), modbusValue.get_value().get_int());
			update = true;
			value = modbusValue.get_value();
		}
		timePoint = chrono::system_clock::now();
	}
	if(update)
	{
		this->update_value_to_subscribers(value, timePoint);
	}
}

ModbusProcess::CoilStatusVariable::CoilStatusVariable(ModbusProcess& _master, uint8_t _slaveAddress, uint16_t _coilAddress) : master(_master)
{
	slaveAddress = _slaveAddress;
	coilAddress = _coilAddress;
}

ModbusProcess::CoilStatusVariable::~CoilStatusVariable()
{

}

void ModbusProcess::CoilStatusVariable::set_value(const Value& newValue)
{
	value = !!newValue.get_int();
	master.add_force_coil_status(slaveAddress, coilAddress, value);
}

Value ModbusProcess::CoilStatusVariable::get_value() const
{
	return value;
}

std::pair<Value, std::chrono::time_point<std::chrono::system_clock>> ModbusProcess::CoilStatusVariable::get_value_with_time() const
{
	lock_guard<mutex> locker(valueLock);
	bool retVal = value;
	auto retVal2 = timePoint;
	return {retVal, retVal2};
}

/*
void ModbusProcess::CoilStatusVariable::add_value_change_subscriber(ModbusSubscriber::CoilStatus& subscriber)
{
	subscribers.push_back(&subscriber);
}
*/
