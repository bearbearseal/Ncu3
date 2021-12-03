#include "ModbusRtu.h"
#include <stdarg.h>

using namespace std;

ModbusRtu::ReplyData::ReplyData()
{
	functionCode = 0;
	slaveAddress = 0;
	data = nullptr;
}

ModbusRtu::ReplyData::ReplyData(const ReplyData& theOther)
{
	//delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_DIGITAL_INPUT_CODE:
		data = new vector<bool>(*((vector<bool>*) theOther.data));
		break;
	case READ_HOLDING_REGISTER_CODE:
	case READ_INPUT_REGISTER_CODE:
		data = new vector<RegisterValue>(*((vector<RegisterValue>*) theOther.data));
		break;
	case FORCE_SINGLE_COIL_CODE:
		data = new pair<uint16_t, bool>(*((pair<uint16_t, bool>*) theOther.data));
		break;
	case PRESET_SINGLE_REGISTER_CODE:
		data = new pair<uint16_t, RegisterValue>(*((pair<uint16_t, RegisterValue>*) theOther.data));
		break;
	case FORCE_MULTIPLE_COILS_CODE:
	case PRESET_MULTIPLE_REGISTERS_CODE:
		data = new pair<uint16_t, uint16_t>(*((pair<uint16_t, uint16_t>*) theOther.data));
		break;
	}
}

ModbusRtu::ReplyData::ReplyData(ReplyData&& theOther)
{
	//delete_data();
	functionCode = theOther.functionCode;
	theOther.functionCode = 0;
	slaveAddress = theOther.slaveAddress;
	data = theOther.data;
	theOther.data = nullptr;
}

void ModbusRtu::ReplyData::operator=(const ReplyData& theOther)
{
	delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_DIGITAL_INPUT_CODE:
		data = new vector<bool>(*((vector<bool>*) theOther.data));
		break;
	case READ_HOLDING_REGISTER_CODE:
	case READ_INPUT_REGISTER_CODE:
		data = new vector<RegisterValue>(*((vector<RegisterValue>*) theOther.data));
		break;
	case FORCE_SINGLE_COIL_CODE:
		data = new pair<uint16_t, bool>(*((pair<uint16_t, bool>*) theOther.data));
		break;
	case PRESET_SINGLE_REGISTER_CODE:
		data = new pair<uint16_t, RegisterValue>(*((pair<uint16_t, RegisterValue>*) theOther.data));
		break;
	case FORCE_MULTIPLE_COILS_CODE:
	case PRESET_MULTIPLE_REGISTERS_CODE:
		data = new pair<uint16_t, uint16_t>(*((pair<uint16_t, uint16_t>*) theOther.data));
		break;
	}
}

void ModbusRtu::ReplyData::operator=(ReplyData&& theOther)
{
	delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	theOther.functionCode = 0;
	data = theOther.data;
	theOther.data = nullptr;
}

void ModbusRtu::ReplyData::delete_data()
{
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_DIGITAL_INPUT_CODE:
		delete ((vector<bool>*) data);
		break;
	case READ_HOLDING_REGISTER_CODE:
	case READ_INPUT_REGISTER_CODE:
		delete ((vector<RegisterValue>*) data);
		break;
	case FORCE_SINGLE_COIL_CODE:
		delete ((pair<uint16_t, bool>*) data);
		break;
	case PRESET_SINGLE_REGISTER_CODE:
		delete ((pair<uint16_t, RegisterValue>*) data);
		break;
	case FORCE_MULTIPLE_COILS_CODE:
	case PRESET_MULTIPLE_REGISTERS_CODE:
		delete ((pair<uint16_t, uint16_t>*) data);
		break;
	}
}

ModbusRtu::ReplyData::~ReplyData()
{
	delete_data();
}

vector<bool>& ModbusRtu::ReplyData::get_coils() const
{
	return *((vector<bool>*) data);
}

vector<bool>& ModbusRtu::ReplyData::get_input_status() const
{
	return *((vector<bool>*) data);
}

vector<RegisterValue>& ModbusRtu::ReplyData::get_holding_register() const
{
	return *((vector<RegisterValue>*) data);
}

vector<RegisterValue>& ModbusRtu::ReplyData::get_input_register() const
{
	return *((vector<RegisterValue>*) data);
}

pair<uint16_t, bool> ModbusRtu::ReplyData::get_force_single_coil() const
{
	return *((pair<uint16_t, bool>*) data);
}

pair<uint16_t, RegisterValue> ModbusRtu::ReplyData::get_preset_single_register() const
{
	return *((pair<uint16_t, RegisterValue>*) data);
}

pair<uint16_t, uint16_t> ModbusRtu::ReplyData::get_force_multiple_coils() const
{
	return *((pair<uint16_t, uint16_t>*) data);
}

pair<uint16_t, uint16_t> ModbusRtu::ReplyData::get_preset_multiple_registers() const
{
	return *((pair<uint16_t, uint16_t>*) data);
}

int16_t ModbusRtu::get_crc(const char* data, uint16_t length, uint16_t poly)
{
	uint16_t crc = 0xFFFF;
	const uint8_t* uData = (const uint8_t*) data;

	for (int pos = 0; pos < length; pos++) 
	{
		crc ^= (uint16_t)uData[pos];          // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) // Loop over each bit
		{    
			if ((crc & 0x0001) != 0) 
			{      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= poly;
			}
			else // Else LSB is not set
			{
				crc >>= 1;                    // Just shift right
			}
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

pair<string, uint16_t> ModbusRtu::create_read_coil_status(uint8_t slaveAddress, uint16_t firstCoilAddress, uint16_t totalCoil)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&READ_COIL_CODE, sizeof(READ_COIL_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(firstCoilAddress >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)firstCoilAddress;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(totalCoil >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)totalCoil;
	retVal.append((char*)&aByte, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t) retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	uint16_t replyLength = 5 + (totalCoil + 7)/8;
	return { retVal, replyLength };
}

pair<string, uint16_t> ModbusRtu::create_read_digital_input(uint8_t slaveAddress, uint16_t firstAddress, uint16_t inputCount)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&READ_DIGITAL_INPUT_CODE, sizeof(READ_DIGITAL_INPUT_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(firstAddress >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)firstAddress;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(inputCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)inputCount;
	retVal.append((char*)&aByte, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	uint16_t replyLength = 5 + (inputCount + 7) / 8;
	return { retVal, replyLength };
}

pair<string, uint16_t> ModbusRtu::create_read_holding_register(uint8_t slaveAddress, uint16_t firstAddress, uint16_t registerCount)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&READ_HOLDING_REGISTER_CODE, sizeof(READ_HOLDING_REGISTER_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(firstAddress >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)firstAddress;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(registerCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)registerCount;
	retVal.append((char*)&aByte, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	uint16_t replyLength = 5 + registerCount * 2;
	return { retVal, replyLength };
}

pair<string, uint16_t> ModbusRtu::create_read_input_register(uint8_t slaveAddress, uint16_t firstAddress, uint16_t dataCount)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&READ_INPUT_REGISTER_CODE, sizeof(READ_INPUT_REGISTER_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(firstAddress >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)firstAddress;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(dataCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)dataCount;
	retVal.append((char*)&aByte, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	uint16_t replyLength = 5 + dataCount * 2;
	return { retVal, replyLength };
}

pair<string, uint16_t> ModbusRtu::create_force_single_coil(uint8_t slaveAddress, uint16_t address, bool value)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&FORCE_SINGLE_COIL_CODE, sizeof(FORCE_SINGLE_COIL_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(address >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)address;
	retVal.append((char*)&aByte, 1);
	if (value)
	{
		aByte = 0xff;
	}
	else
	{
		aByte = 0x00;
	}
	retVal.append((char*)&aByte, 1);
	aByte = 0x00;
	retVal.append((char*)&aByte, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	return { retVal, 8 };
}

pair<string, uint16_t> ModbusRtu::create_preset_single_register(uint8_t slaveAddress, uint16_t address, RegisterValue& value)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&PRESET_SINGLE_REGISTER_CODE, sizeof(PRESET_SINGLE_REGISTER_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(address >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)address;
	retVal.append((char*)&aByte, 1);
	retVal.append(value.high, 1);
	retVal.append(value.low, 1);
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	return { retVal, 8 };
}

//VA_LIST shall be a list of uint8_t
pair<string, uint16_t> ModbusRtu::create_force_multiple_coils(uint8_t slaveAddress, uint16_t address, uint16_t coilCount, ...)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&FORCE_MULTIPLE_COILS_CODE, sizeof(FORCE_MULTIPLE_COILS_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(address >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)address;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(coilCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)coilCount;
	retVal.append((char*)&aByte, 1);
	uint8_t dataByteCount = coilCount / 8;
	if (coilCount%8>0)
	{
		++dataByteCount;
	}
	retVal.append((char*)&dataByteCount, 1);
	va_list arg;
	va_start(arg, coilCount);
	for (unsigned i = 0; i < dataByteCount; ++i)
	{
		aByte = (uint8_t) va_arg(arg, int);
		retVal.append((char*)&aByte, 1);
	}
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	return { retVal,8 };
}

//VA_LIST shall be a list of uint16_t
pair<string, uint16_t> ModbusRtu::create_preset_multiple_registers(uint8_t slaveAddress, uint16_t address, uint16_t registerCount, ...)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&PRESET_MULTIPLE_REGISTERS_CODE, sizeof(PRESET_MULTIPLE_REGISTERS_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(address >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)address;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(registerCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)registerCount;
	retVal.append((char*)&aByte, 1);
	uint8_t dataByteCount = (uint8_t) registerCount*2;
	retVal.append((char*)&dataByteCount, 1);
	va_list arg;
	va_start(arg, registerCount);
	for (unsigned i = 0; i < registerCount; ++i)
	{
		uint16_t aRegister = (uint16_t) va_arg(arg, int);
		aByte = (uint8_t)(aRegister >> 8);
		retVal.append((char*)&aByte, 1);
		aByte = (uint8_t)aRegister;
		retVal.append((char*)&aByte, 1);
	}
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	return { retVal, 8 };
}

pair<string, uint16_t> ModbusRtu::create_preset_multiple_registers(uint8_t slaveAddress, uint16_t address, const vector<RegisterValue>& newValues)
{
	string retVal;
	retVal.append((char*)&slaveAddress, sizeof(slaveAddress));
	retVal.append((char*)&PRESET_MULTIPLE_REGISTERS_CODE, sizeof(PRESET_MULTIPLE_REGISTERS_CODE));
	uint8_t aByte;
	aByte = (uint8_t)(address >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)address;
	retVal.append((char*)&aByte, 1);
	uint16_t registerCount = newValues.size();
	aByte = (uint8_t)(registerCount >> 8);
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)registerCount;
	retVal.append((char*)&aByte, 1);
	uint8_t dataByteCount = (uint8_t)registerCount * 2;
	retVal.append((char*)&dataByteCount, 1);
	//printf("Create multiple size: %u\n", registerCount);
	for (unsigned i = 0; i < registerCount; ++i)
	{
		retVal.append((char*)&(newValues[i].high), 1);
		retVal.append((char*)&(newValues[i].low), 1);
	}
	uint16_t crc = get_crc(retVal.c_str(), (uint16_t)retVal.size(), MODBUS_POLY);
	aByte = (uint8_t)crc;
	retVal.append((char*)&aByte, 1);
	aByte = (uint8_t)(crc >> 8);
	retVal.append((char*)&aByte, 1);
	return { retVal, 8 };
}

ModbusRtu::ReplyData ModbusRtu::decode_reply(const string& reply)
{
	ReplyData retVal;
	if (reply.size() < 5)
	{
		retVal.functionCode = 0;
		return retVal;
	}
	uint16_t crc = get_crc(reply.c_str(), (uint16_t)reply.size()-2, MODBUS_POLY);
	uint8_t crcHigh = (uint8_t)(crc >> 8);
	uint8_t crcLow = (uint8_t)crc;
	if (((uint8_t) reply[reply.size() - 2]) != crcLow || ((uint8_t)reply[reply.size() - 1]) != crcHigh)
	{
		printf("Crc Error: [%02X%02X] [%02X%02X]\n", ((uint8_t)reply[reply.size() - 2]), ((uint8_t)reply[reply.size() - 1]), crcLow, crcHigh);
		retVal.functionCode = 0;
		return retVal;
	}
	retVal.slaveAddress = reply[0];
	retVal.functionCode = reply[1];
	switch (retVal.functionCode)
	{
	case READ_COIL_CODE:
	case READ_DIGITAL_INPUT_CODE:
	{
		uint8_t bytesCount = reply[2];
		if (bytesCount != (reply.size() - 5))
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new vector<bool>();
		vector<bool>& data = *((vector<bool>*) retVal.data);
		for (unsigned i = 0; i < bytesCount; ++i)
		{
			for (unsigned j = 0; j < 8; ++j)
			{
				data.push_back((reply[3+i]>>j)&0x01);
			}
		}
		break;
	}
	case READ_HOLDING_REGISTER_CODE:
	case READ_INPUT_REGISTER_CODE:
	{
		uint8_t bytesCount = reply[2];
		if (bytesCount != (reply.size()-5))
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new vector<RegisterValue>();
		vector<RegisterValue>& data = *((vector<RegisterValue>*) retVal.data);
		for (unsigned i = 0; i < bytesCount; i += 2)
		{
			RegisterValue aValue;
			aValue.high = (uint8_t) reply[3 + i];
			aValue.low = (uint8_t)reply[4 + i];
			data.push_back(aValue);
		}
		break;
	}
	case FORCE_SINGLE_COIL_CODE:
	{
		if (reply.size() != 8)
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new pair<uint16_t, bool>();
		pair<uint16_t, bool>& data = *((pair<uint16_t, bool>*) retVal.data);
		data.first = (reply[2] & 0xff);
		data.first <<= 8;
		data.first += (reply[3] & 0xff);
		data.second = (reply[4] == 0xff);
		break;
	}
	case PRESET_SINGLE_REGISTER_CODE:
	{
		if (reply.size() != 8)
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new pair<uint16_t, RegisterValue>();
		pair<uint16_t, RegisterValue>& data = *((pair<uint16_t, RegisterValue>*) retVal.data);
		data.first = (reply[2] & 0xff);
		data.first <<= 8;
		data.first += (reply[3] & 0xff);
		data.second.high = reply[4];
		data.second.low = reply[5];
		break;
	}
	case FORCE_MULTIPLE_COILS_CODE:
	case PRESET_MULTIPLE_REGISTERS_CODE:
	{
		if (reply.size() != 8)
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new pair<uint16_t, uint16_t>();
		pair<uint16_t, uint16_t>& data = *((pair<uint16_t, uint16_t>*) retVal.data);
		data.first = (reply[2] & 0xff);
		data.first <<= 8;
		data.first += (reply[3] & 0xff);
		data.second = (reply[4] & 0xff);
		data.second <<= 8;
		data.second += (reply[5] & 0xff);
		break;
	}
	default:
		retVal.functionCode = 0;
		break;
	}
	return retVal;
}
