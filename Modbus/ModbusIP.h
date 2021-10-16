#ifndef _MODBUSIP_H_
#define _MODBUSIP_H_
#include "ModbusRegisterValue.h"
#include <vector>
#include <utility>

namespace ModbusIP{

	const uint8_t READ_COIL_CODE = 0x01;
	const uint8_t READ_INPUT_CODE = 0x02;
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
		uint16_t sequenceNumber;
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
		void operator=(const ReplyData& theOther);
		void operator=(ReplyData&& theOther);

	private:
		void delete_data();
	};

	//the query string + the expected reply length
    std::pair<std::string, uint16_t> construct_read_coils(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t coilAddress, uint16_t count);
    std::pair<std::string, uint16_t> construct_write_single_coil(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t coilAddress, bool value);
	std::pair<std::string, uint16_t> construct_read_digital_input(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t inputAddress, uint16_t count);
    std::pair<std::string, uint16_t> construct_read_input_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t count);
    std::pair<std::string, uint16_t> construct_read_holding_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t count);
    std::pair<std::string, uint16_t> construct_write_single_holding_register(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t value);
    std::pair<std::string, uint16_t> construct_write_multiple_holding_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, const std::vector<RegisterValue>& values);


	ReplyData decode_reply(const std::string& reply);

}
#endif