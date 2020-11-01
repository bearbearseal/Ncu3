#ifndef _MODBUSIPPROCESS_H_
#define _MODBUSIPPROCESS_H_
#include <thread>
#include <mutex>
#include <map>
#include <chrono>
#include "ModbusIP.h"
#include "../../MyLib/TcpSocket/TcpSocket.h"
#include "../../MyLib/Basic/Variable.h"

class ModbusIpProcess {
private:
	class Shadow;

public:
    ModbusIpProcess(const std::string& hisAddress, uint16_t hisPort, uint8_t slaveAddress, uint16_t maxRegister, uint16_t maxCoilPerMessage, bool smallEndian, std::chrono::milliseconds timeout);
    ~ModbusIpProcess();

	void set_behavior();
    void start();
    void stop();

	class CoilStatusVariable : public Variable {
		friend class ModbusIpProcess;
	public:
		CoilStatusVariable(std::shared_ptr<Shadow> _master, uint16_t _coilAddress);
		virtual ~CoilStatusVariable();

	private:
		virtual void _write_value(const Value& newValue);
		void update_value_from_source(uint16_t firstAddress, const std::vector<bool>& values);
		std::weak_ptr<Shadow> master;
		uint16_t coilAddress;
	};
	std::shared_ptr<CoilStatusVariable> get_coil_status_variable(uint16_t coilAddress);

	class HoldingRegisterVariable : public Variable {
		friend class ModbusIpProcess;
	public:
		HoldingRegisterVariable(
			std::shared_ptr<Shadow> _master,
			uint16_t _firstAddress,
			ModbusRegisterValue::DataType _type,
			bool smallEndian);
		virtual ~HoldingRegisterVariable();

	private:
		void update_value_from_source(uint16_t _registerAddress, const std::vector<RegisterValue>& values);
		inline uint8_t register_count() { return ModbusRegisterValue::get_register_count(type); }
		virtual void _write_value(const Value& newValue);

		std::weak_ptr<Shadow> master;
		uint16_t firstAddress;
		bool smallEndian;
		ModbusRegisterValue::DataType type;
	};
	std::shared_ptr<HoldingRegisterVariable> get_holding_register_variable(uint16_t registerAddress, ModbusRegisterValue::DataType type);

private:
	std::shared_ptr<Shadow> myShadow;
	struct HoldingRegisterQueryData{
		uint16_t startAddress;
		uint16_t registerCount;
		std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
	};
	std::vector<HoldingRegisterQueryData> holdingRegisterQueryList;

	struct CoilQueryData{
		uint16_t startAddress;
		uint16_t coilCount;
		std::vector<std::shared_ptr<CoilStatusVariable>> variables;
	};
	std::vector<CoilQueryData> coilQueryList;

    std::map<uint16_t, std::shared_ptr<CoilStatusVariable>> address2CoilMap;
    struct RegisterData{
        size_t count = 0;   //longest variables register count 
        std::vector<std::shared_ptr<HoldingRegisterVariable>> variables;
    };
    std::map<uint16_t, RegisterData> address2HoldingRegisterMap;

	void convert_variable_map_to_query();

    struct{
        uint8_t slaveAddress;
        uint16_t maxRegisterPerMessage;
        uint16_t maxCoilPerMessage;
        bool isSmallEndian;
		std::chrono::duration<int, std::milli> timeout;
    }config;

    TcpSocket socket;
    struct{
        std::mutex threadMutex;
        bool keepRunning = false;
        std::thread* theProcess = nullptr;
        uint8_t mainState;
		uint8_t subState;
		uint8_t failCount;
    }threadData;

	struct ForceCoilData {
		std::mutex mapMutex;
		std::unordered_map<uint16_t, bool> valueMap;
	}forceCoilData;

	struct WriteHoldingRegisterData {
		std::mutex mapMutex;
		std::map<uint16_t, std::vector<RegisterValue>> valueMap;
	}writeHoldingRegisterData;
	

    static void thread_process(ModbusIpProcess* me);

	void force_coil(uint16_t coilAddress, bool value);
	void write_multiple_holding_register(uint16_t registerAddress, const std::vector<RegisterValue>& values);
	std::pair<bool, std::string> query_holding_register_then_get_reply(const HoldingRegisterQueryData& queryData, uint16_t sequenceNumber, std::chrono::milliseconds waitTime);
	bool do_write_coil_query(uint16_t& sequenceNumber, std::chrono::milliseconds waitTime);
	bool do_write_holding_register_query(uint16_t& sequnceNumber, std::chrono::milliseconds waitTime);

	class Shadow {
		friend class CoilStatusVariable;
		friend class HoldingRegisterVariable;
	public:
		Shadow(ModbusIpProcess& _real) : real(_real) {}
		virtual ~Shadow() {}
		void force_coil(uint16_t coilAddress, bool value) { real.force_coil(coilAddress, value); }
		void write_multiple_holding_register(uint16_t registerAddress, const std::vector<RegisterValue>& values) { real.write_multiple_holding_register(registerAddress, values); }
		std::pair<bool, std::string> query_holding_register_then_get_reply(const HoldingRegisterQueryData& queryData, uint16_t sequenceNumber, std::chrono::milliseconds waitTime)
			{ return real.query_holding_register_then_get_reply(queryData, sequenceNumber, waitTime); }
		bool do_write_coil_query(uint16_t& sequenceNumber, std::chrono::milliseconds waitTime) { return real.do_write_coil_query(sequenceNumber, waitTime); }
		bool do_write_holding_register_query(uint16_t& sequenceNumber, std::chrono::milliseconds waitTime) { return real.do_write_holding_register_query(sequenceNumber, waitTime); }
	private:
		ModbusIpProcess& real;
	};
};
#endif
