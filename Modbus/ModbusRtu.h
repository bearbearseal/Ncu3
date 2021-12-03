#ifndef _Modbustu_H_
#define _ModbusRtu_H_
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>
#include "ModbusDefinition.h"

//Modbus uses little endian

namespace ModbusRtu
{
	const uint16_t MODBUS_POLY = 0xA001;
	const uint8_t READ_COIL_CODE = 0x01;
	const uint8_t READ_DIGITAL_INPUT_CODE = 0x02;
	const uint8_t READ_HOLDING_REGISTER_CODE = 0x03;
	const uint8_t READ_INPUT_REGISTER_CODE = 0x04;
	const uint8_t FORCE_SINGLE_COIL_CODE = 0x05;
	const uint8_t PRESET_SINGLE_REGISTER_CODE = 0x06;
	const uint8_t FORCE_MULTIPLE_COILS_CODE = 0x0F;
	const uint8_t PRESET_MULTIPLE_REGISTERS_CODE = 0x10;

	struct ReplyData
	{
		uint8_t functionCode;
		uint8_t slaveAddress;
		void* data;

		ReplyData();
		ReplyData(const ReplyData& theOther);
		ReplyData(ReplyData&& theOther);
		~ReplyData();
		std::vector<bool>& get_coils() const;
		std::vector<bool>& get_input_status() const;
		std::vector<RegisterValue>& get_holding_register() const;
		std::vector<RegisterValue>& get_input_register() const;
		std::pair<uint16_t, bool> get_force_single_coil() const;
		std::pair<uint16_t, RegisterValue> get_preset_single_register() const;
		std::pair<uint16_t, uint16_t> get_force_multiple_coils() const;
		std::pair<uint16_t, uint16_t> get_preset_multiple_registers() const;
		//ReplyData& operator=(const ReplyData& theOther);
		void operator=(const ReplyData& theOther);
		//ReplyData& operator=(ReplyData&& theOther);
		void operator=(ReplyData&& theOther);

	private:
		void delete_data();
	};

	int16_t get_crc(const char* data, uint16_t length, uint16_t poly);

	//the query string + the expected reply length
	std::pair<std::string, uint16_t> create_read_coil_status(uint8_t slaveAddress, uint16_t firstCoilAddress, uint16_t totalCoil);

	std::pair<std::string, uint16_t> create_read_digital_input(uint8_t slaveAddress, uint16_t firstAddress, uint16_t inputCount);

	std::pair<std::string, uint16_t> create_read_holding_register(uint8_t slaveAddress, uint16_t firstAddress, uint16_t registerCount);

	std::pair<std::string, uint16_t> create_read_input_register(uint8_t slaveAddress, uint16_t firstAddress, uint16_t dataCount);

	std::pair<std::string, uint16_t> create_force_single_coil(uint8_t slaveAddress, uint16_t address, bool value);

	std::pair<std::string, uint16_t> create_preset_single_register(uint8_t slaveAddress, uint16_t address, RegisterValue& value);

	//VA_LIST shall be a list of uint8_t
	std::pair<std::string, uint16_t> create_force_multiple_coils(uint8_t slaveAddress, uint16_t address, uint16_t coilCount, ...);

	//VA_LIST shall be a list of uint16_t
	std::pair<std::string, uint16_t> create_preset_multiple_registers(uint8_t slaveAddress, uint16_t address, uint16_t registerCount, ...);
	std::pair<std::string, uint16_t> create_preset_multiple_registers(uint8_t slaveAddress, uint16_t address, const std::vector<RegisterValue>& newValues);

	ReplyData decode_reply(const std::string& reply);
}
#endif
