#ifndef _ModbusRtuProcess_H_
#define _ModbusRtuProcess_H_
#include "../../MyLib/SerialPort/SyncedSerialPort.h"
#include "../../MyLib/Basic/Variable.h"
#include "../InOutOperation/OperationVariable.h"
#include "ModbusRegisterValue.h"
#include <map>
#include <vector>
#include <unordered_map>
#include <chrono>

class ModbusRtuProcess
{
private:
	// Proxy of ModbusRtuProcess
	class Shadow
	{
	public:
		Shadow(ModbusRtuProcess &_master) : master(_master) {}
		virtual ~Shadow() {}
		void add_write_holding_register(uint16_t registerAddress, std::vector<RegisterValue> &registerValues) { master.add_write_holding_register(registerAddress, registerValues); }
		void add_force_coil_status(uint16_t coilAddress, bool newValue) { master.add_force_coil_status(coilAddress, newValue); }

	private:
		ModbusRtuProcess &master;
	};

public:
	class CoilStatusVariable : public OperationVariable
	{
		friend class ModbusRtuProcess;

	private:
		CoilStatusVariable(
			std::shared_ptr<Shadow> _master,
			uint16_t _coilAddress,
			std::shared_ptr<OperationalLogic> inLogic,
			std::shared_ptr<OperationalLogic> outLogic);

	public:
		virtual ~CoilStatusVariable();

	private:
		bool _write_value(const Value &newValue);
		void update_value_from_source(uint16_t firstAddress, const std::vector<bool> &values);
		std::weak_ptr<Shadow> master;
		uint16_t coilAddress;
	};
	std::shared_ptr<CoilStatusVariable> get_coil_status_variable(uint16_t coilAddress, std::shared_ptr<OperationalLogic> inLogic = nullptr, std::shared_ptr<OperationalLogic> outLogic = nullptr);

	class DigitalInputVariable : public OperationVariable
	{
		friend class ModbusRtuProcess;

	private:
		DigitalInputVariable(
			std::shared_ptr<Shadow> _master,
			uint16_t _address,
			std::shared_ptr<OperationalLogic> inLogic,
			std::shared_ptr<OperationalLogic> outLogic);

	public:
		virtual ~DigitalInputVariable();

	private:
		bool _write_value(const Value &newValue) { return false; }
		void update_value_from_source(uint16_t firstAddress, const std::vector<bool> &values);
		std::weak_ptr<Shadow> master;
		uint16_t address;
	};
	std::shared_ptr<DigitalInputVariable> get_digital_input_variable(uint16_t address, std::shared_ptr<OperationalLogic> inLogic = nullptr, std::shared_ptr<OperationalLogic> outLogic = nullptr);

	class HoldingRegisterVariable : public OperationVariable
	{
		friend class ModbusRtuProcess;

	private:
		HoldingRegisterVariable(
			std::shared_ptr<Shadow> _master,
			uint16_t _firstAddress,
			ModbusRegisterValue::DataType _type,
			bool _isSmallEndian,
			std::shared_ptr<OperationalLogic> inLogic,
			std::shared_ptr<OperationalLogic> outLogic);

	public:
		virtual ~HoldingRegisterVariable();

	private:
		bool _write_value(const Value &newValue);
		void update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue> &values);
		inline uint8_t register_count() { return ModbusRegisterValue::get_register_count(type); }

		std::weak_ptr<Shadow> master;
		bool isSmallEndian;
		uint16_t firstAddress;
		ModbusRegisterValue::DataType type;
	};
	std::shared_ptr<HoldingRegisterVariable> get_holding_register_variable(uint16_t registerAddress, ModbusRegisterValue::DataType type, std::shared_ptr<OperationalLogic> inLogic = nullptr, std::shared_ptr<OperationalLogic> outLogic = nullptr);

	class InputRegisterVariable : public OperationVariable
	{
		friend class ModbusRtuProcess;

	private:
		InputRegisterVariable(
			std::shared_ptr<Shadow> _master,
			uint16_t _firstAddress,
			ModbusRegisterValue::DataType _type,
			bool _smallEndian,
			std::shared_ptr<OperationalLogic> inLogic,
			std::shared_ptr<OperationalLogic> outLogic);

	public:
		virtual ~InputRegisterVariable();

	private:
		bool _write_value(const Value &newValue) { return false; }
		void update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue> &values);
		inline uint8_t register_count() { return ModbusRegisterValue::get_register_count(type); }

		std::weak_ptr<Shadow> master;
		uint16_t firstAddress;
		bool isSmallEndian;
		ModbusRegisterValue::DataType type;
	};
	std::shared_ptr<InputRegisterVariable> get_input_register_variable(uint16_t registerAddress, ModbusRegisterValue::DataType type, std::shared_ptr<OperationalLogic> inLogic = nullptr, std::shared_ptr<OperationalLogic> outLogic = nullptr);

	ModbusRtuProcess(std::shared_ptr<SyncedSerialPort> serialPort, uint8_t slaveAddress, uint16_t maxRegisterPerMessage, uint16_t maxCoilPerMessage, std::chrono::milliseconds timeout, bool smallEndian);
	~ModbusRtuProcess();

	void start();
	void stop();

private:
	std::shared_ptr<SyncedSerialPort> serialPort;
	std::shared_ptr<Shadow> myShadow;

	struct
	{
		uint8_t slaveAddress = 1;
		uint16_t maxRegisterPerMessage = 16;
		uint16_t maxCoilPerMessage = 64;
		std::chrono::milliseconds timeout = std::chrono::milliseconds(250);
		bool smallEndian = false;
	} config;
	struct
	{
		std::mutex theMutex;
		std::unordered_map<uint32_t, std::vector<RegisterValue>> address2ValueWriteMap;
		std::vector<uint32_t> writeOrder;
	} writeHoldingRegisterData;
	struct
	{
		std::mutex theMutex;
		std::map<uint16_t, bool> address2ValueWriteMap;
	} forceCoilData;
	struct HoldingRegisterData
	{
		uint8_t count = 0;
		std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
	};
	struct InputRegisterData
	{
		uint8_t count = 0;
		std::vector<std::shared_ptr<InputRegisterVariable>> variables;
	};
	std::mutex holdingRegisterVariableMutex;
	std::map<uint16_t, HoldingRegisterData> address2HoldingRegisterVariableMap;
	std::mutex inputRegisterVariableMutex;
	std::map<uint16_t, InputRegisterData> address2InputRegisterVariableMap;
	std::mutex coilVariableMutex;
	std::map<uint16_t, std::shared_ptr<CoilStatusVariable>> address2CoilVariableMap;
	std::mutex digitalInputMutex;
	std::map<uint16_t, std::shared_ptr<DigitalInputVariable>> address2DigitalVariableMap;
	struct HoldingRegisterQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
	};
	size_t holdingRegisterQueryIndex = 0;
	std::vector<HoldingRegisterQuery> holdingRegisterQueryList;
	struct InputRegisterQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<InputRegisterVariable>> variables;
	};
	size_t inputRegisterQueryIndex = 0;
	std::vector<InputRegisterQuery> inputRegisterQueryList;

	struct CoilStatusQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<CoilStatusVariable>> variables;
	};
	size_t coilStatusQueryIndex = 0;
	std::vector<CoilStatusQuery> coilStatusQueryList;
	struct DigitalInputQuery
	{
		std::string query;
		uint16_t replyLength;
		uint16_t startAddress;
		std::vector<std::shared_ptr<DigitalInputVariable>> variables;
	};
	size_t digitalInputQueryInput = 0;
	std::vector<DigitalInputQuery> digitalInputQueryList;

	void add_write_holding_register(uint16_t registerAddress, std::vector<RegisterValue> &registerValues);
	void write_holding_register();
	void add_force_coil_status(uint16_t coilAddress, bool newValue);
	void force_coil_status();
	void reset_holding_register_query() { holdingRegisterQueryIndex = 0; }
	void reset_coil_status_query() { coilStatusQueryIndex = 0; }
	void build_query();
	void clean_build_data();

	bool keepRunning;
	std::unique_ptr<std::thread> theThread;
	static void thread_process(ModbusRtuProcess *theProcess);
};

#endif
