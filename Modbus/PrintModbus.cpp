#include "PrintModbus.h"

void PrintModbus::print_value_in_hex(const std::string& source)
{
	for (unsigned i = 0; i < source.size(); ++i)
	{
		printf("%02X", (uint8_t)(source[i] & 0xff));
	}
	printf("\n");
}

void PrintModbus::print_modbus_reply(const ModbusRtu::ReplyData& data)
{
	printf("Slave Address: %u\nFunction Code: %u\n", data.slaveAddress, data.functionCode);
	switch (data.functionCode)
	{
	case ModbusRtu::READ_COIL_CODE:
	{
		auto coilsStatus = data.get_coils();
		for (unsigned i = 0; i < coilsStatus.size(); ++i)
		{
			if (coilsStatus[i])
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
		}
		break;
	}
	case ModbusRtu::READ_INPUT_CODE:
	{
		auto inputStatus = data.get_input_status();
		for (unsigned i = 0; i < inputStatus.size(); ++i)
		{
			if (inputStatus[i])
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
		}
		break;
	}
	case ModbusRtu::READ_HOLDING_REGISTER_CODE:
	{
		auto holdingRegister = data.get_holding_register();
		for (unsigned i = 0; i < holdingRegister.size(); ++i)
		{
			printf("[%02X%02X]", holdingRegister[i].high, holdingRegister[i].low);
		}
		break;
	}
	case ModbusRtu::READ_INPUT_REGISTER_CODE:
	{
		auto inputRegister = data.get_input_register();
		for (unsigned i = 0; i < inputRegister.size(); ++i)
		{
			printf("%02X%02X", inputRegister[i].high, inputRegister[i].low);
		}
		break;
	}
	case ModbusRtu::FORCE_SINGLE_COIL_CODE:
	{
		auto forceSingle = data.get_force_single_coil();
		printf("Address:%04X Value:%s", forceSingle.first, forceSingle.second ? "on" : "off");
		break;
	}
	case ModbusRtu::PRESET_SINGLE_REGISTER_CODE:
	{
		auto presetSingle = data.get_preset_single_register();
		printf("Address:%04X Value:%02X%02X", presetSingle.first, presetSingle.second.high, presetSingle.second.low);
		break;
	}
	case ModbusRtu::FORCE_MULTIPLE_COILS_CODE:
	{
		auto forceMultiple = data.get_force_multiple_coils();
		printf("Address:%04X Count:%d\n", forceMultiple.first, forceMultiple.second);
		break;
	}
	case ModbusRtu::PRESET_MULTIPLE_REGISTERS_CODE:
	{
		auto presetMultiple = data.get_preset_multiple_registers();
		printf("Address:%04X Count:%d\n", presetMultiple.first, presetMultiple.second);
		break;
	}
	}
	printf("\n");
}
