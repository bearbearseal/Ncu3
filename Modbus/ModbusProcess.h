#ifndef _MODBUSPROCESS_H_
#define _MODBUSPROCESS_H_
#include "../Basic/SerialPort.h"
#include "ModbusRTU.h"
#include "../Basic/Variable.h"
#include "ModbusSubscriber.h"
#include "ModbusRegisterValue.h"
#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>

class ModbusProcess
{
public:
	ModbusProcess(SerialPort& _serialPort);
	~ModbusProcess();

	void start();
	void stop(){}
	bool set_slave(uint8_t address, uint16_t maxRegister, uint16_t maxCoilPerMessage, bool smallEndian);

	class CoilStatusVariable : public Variable
	{
		friend class ModbusProcess;
	public:
		CoilStatusVariable(ModbusProcess& _master, uint8_t _slaveAddress, uint16_t _coilAddress);
		virtual ~CoilStatusVariable();
		void set_value(const Value& newValue);
		Value get_value() const;
		std::pair<Value, std::chrono::time_point<std::chrono::system_clock>> get_value_with_time() const;

	private:
		void update_value_from_source(uint16_t firstAddress, const std::vector<bool>& values);
		ModbusProcess& master;
		mutable std::mutex valueLock;
		bool value;
		uint8_t slaveAddress;
		uint16_t coilAddress;
		std::chrono::time_point<std::chrono::system_clock> timePoint;
	};
	std::shared_ptr<CoilStatusVariable> get_coil_status_variable(uint8_t slaveAddress, uint16_t coilAddress);

	class HoldingRegisterVariable : public Variable
	{
		friend class ModbusProcess;
	public:
		HoldingRegisterVariable(
			ModbusProcess& _master,
			uint8_t _slaveAddress,
			uint16_t _firstAddress,
			ModbusRegisterValue::DataType _type,
			bool smallEndian);
		virtual ~HoldingRegisterVariable();
		void set_value(const Value& newValue);
		Value get_value() const;
		std::pair<Value, std::chrono::time_point<std::chrono::system_clock>> get_value_with_time() const;

	private:
		void update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue>& values);
		inline uint8_t register_count() { return ModbusRegisterValue::get_register_count(type); }

		ModbusProcess& master;
		mutable std::mutex valueLock;
		Value value;
		uint8_t slaveAddress;
		uint16_t firstAddress;
		bool smallEndian;
		ModbusRegisterValue::DataType type;
		std::chrono::time_point<std::chrono::system_clock> timePoint;
	};
	std::shared_ptr<HoldingRegisterVariable> get_holding_register_variable(
		uint8_t slaveAddress,
		uint16_t registerAddress,
		ModbusRegisterValue::DataType type,
		bool smallEndian = true);

private:
	SerialPort& serialPort;
	uint8_t machineState = 0;

	struct
	{
		std::mutex lock;
		std::unordered_map<uint32_t, std::vector<RegisterValue>> slaveRegister2ValueWriteMap;
		std::vector<uint32_t> writeOrder;
	}writeHoldingRegisterData;
	void add_write_holding_register(uint8_t slaveAddress, uint16_t registerAddress, std::vector<RegisterValue>&& registerValues);
	void write_holding_register();

	struct
	{
		std::mutex lock;
		std::unordered_map<uint8_t, std::map<uint16_t, bool>> slave2CoilAddress2ValueWriteMap;
	}forceCoilData;
	void add_force_coil_status(uint8_t slaveAddress, uint16_t coilAddress, bool newValue);
	void force_coil_status();

	struct HoldingRegisterData
	{
		uint8_t count = 0;
		std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
	};

	struct SlaveData
	{
		//std::chrono::duration<std::chrono::milliseconds> delay;	//Delay between each poll
		struct HoldingRegister
		{
			uint16_t maxPerQuery = 32;	//Number of holding registers allowed per read
			std::map<uint16_t, HoldingRegisterData> address2VariableReadMap;	//Read the values of the addresses and put to the variables
		}holdingRegister;
		struct CoilStatus
		{
			uint16_t maxCoilPerQuery = 32;
			std::map<uint16_t, std::shared_ptr<CoilStatusVariable>> variables;
		}coilStatus;
	};
	std::unordered_map<uint8_t, SlaveData> slaveDataMap;

	struct HoldingRegisterQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
	};
	size_t holdingRegisterQueryIndex = 0;
	std::vector<HoldingRegisterQuery> holdingRegisterQueryList;
	HoldingRegisterQuery* get_next_holding_register_query();
	void reset_holding_register_query() { holdingRegisterQueryIndex = 0; }

	struct CoilStatusQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<CoilStatusVariable>> variables;
	};
	size_t coilStatusQueryIndex = 0;
	std::vector<CoilStatusQuery> coilStatusQueryList;
	CoilStatusQuery* get_next_coil_status_query();
	void reset_coil_status_query() { coilStatusQueryIndex = 0; }

	void build_query();
	void clean_build_data();

	static void thread_process(ModbusProcess* theProcess);

};

#endif
