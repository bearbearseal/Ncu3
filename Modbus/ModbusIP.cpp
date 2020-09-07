#include "ModbusIP.h"

using namespace std;

ModbusIP::ReplyData::ReplyData()
{
	functionCode = 0;
	slaveAddress = 0;
	data = nullptr;
}

ModbusIP::ReplyData::ReplyData(const ReplyData& theOther)
{
	//delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_INPUT_CODE:
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

ModbusIP::ReplyData::ReplyData(ReplyData&& theOther)
{
	//delete_data();
	functionCode = theOther.functionCode;
	theOther.functionCode = 0;
	slaveAddress = theOther.slaveAddress;
	data = theOther.data;
	theOther.data = nullptr;
}

void ModbusIP::ReplyData::operator=(const ReplyData& theOther)
{
	delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_INPUT_CODE:
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

void ModbusIP::ReplyData::operator=(ReplyData&& theOther)
{
	delete_data();
	functionCode = theOther.functionCode;
	slaveAddress = theOther.slaveAddress;
	theOther.functionCode = 0;
	data = theOther.data;
	theOther.data = nullptr;
}

void ModbusIP::ReplyData::delete_data()
{
	switch (functionCode)
	{
	case READ_COIL_CODE:
	case READ_INPUT_CODE:
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

ModbusIP::ReplyData::~ReplyData()
{
	delete_data();
}

vector<bool>& ModbusIP::ReplyData::get_coils() const
{
	return *((vector<bool>*) data);
}

vector<bool>& ModbusIP::ReplyData::get_input_status() const
{
	return *((vector<bool>*) data);
}

vector<RegisterValue>& ModbusIP::ReplyData::get_holding_register() const
{
	return *((vector<RegisterValue>*) data);
}

vector<RegisterValue>& ModbusIP::ReplyData::get_input_register() const
{
	return *((vector<RegisterValue>*) data);
}

pair<uint16_t, bool> ModbusIP::ReplyData::get_force_single_coil() const
{
	return *((pair<uint16_t, bool>*) data);
}

pair<uint16_t, RegisterValue> ModbusIP::ReplyData::get_preset_single_register() const
{
	return *((pair<uint16_t, RegisterValue>*) data);
}

pair<uint16_t, uint16_t> ModbusIP::ReplyData::get_force_multiple_coils() const
{
	return *((pair<uint16_t, uint16_t>*) data);
}

pair<uint16_t, uint16_t> ModbusIP::ReplyData::get_preset_multiple_registers() const
{
	return *((pair<uint16_t, uint16_t>*) data);
}

pair<string, uint16_t> ModbusIP::construct_read_coils(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t coilAddress, uint16_t count){
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)0;	//frame length
	retVal += (char)6;	//frame length
	retVal += slaveAddress;
	retVal += (char)READ_COIL_CODE;	//function code
	retVal += (char)(coilAddress>>8);
	retVal += (char)coilAddress;
	retVal += (char)(count>>8);
	retVal += (char)count;
	uint16_t replyLength;
	//reply:
	//2 bytes sequence number
	//2 bytes protocol number, always 0
	//2 bytes following length
	//1 byte slave address
	//1 byte function code
	//1 byte of data byte count
	//n bytes of data
	replyLength = 9 + count/8;
	if(count%8){
		++replyLength;
	}
	return {retVal, replyLength};
}

pair<string, uint16_t> ModbusIP::construct_write_single_coil(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t coilAddress, bool value){
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)0;	//frame length
	retVal += (char)6;	//frame length
	retVal += slaveAddress;
	retVal += (char)FORCE_SINGLE_COIL_CODE;	//function code
	retVal += (char)(coilAddress>>8);
	retVal += (char)coilAddress;
	if(value){
		retVal += (char)0xff;
	}
	else{
		retVal += (char)0;
	}
	retVal += (char)0;
	uint16_t replyLength = retVal.size();
	//reply:
	//Same as request
	return {retVal, replyLength};
}

pair<string, uint16_t> ModbusIP::construct_read_input_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t count){
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)0;	//frame length
	retVal += (char)6;	//frame length
	retVal += slaveAddress;
	retVal += (char)READ_INPUT_REGISTER_CODE;	//function code
	retVal += (char)(registerAddress>>8);
	retVal += (char)registerAddress;
	retVal += (char)count>>8;
	retVal += (char)count;
	uint16_t replyLength = 9 + count*2;
	//reply:
	//2 bytes sequence number
	//2 bytes protocol number, always 0
	//2 bytes following length
	//1 byte slave address
	//1 byte function code
	//1 byte of data byte count
	//n bytes of data
	return {retVal, replyLength};
}

pair<string, uint16_t> ModbusIP::construct_read_multiple_holding_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t count){
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)0;	//frame length
	retVal += (char)6;	//frame length
	retVal += slaveAddress;
	retVal += (char)READ_HOLDING_REGISTER_CODE;	//function code
	retVal += (char)(registerAddress>>8);
	retVal += (char)registerAddress;
	retVal += (char)(count>>8);
	retVal += (char)count;
	uint16_t replyLength = 9 + count*2;
	//reply:
	//2 bytes sequence number
	//2 bytes protocol number, always 0
	//2 bytes following length
	//1 byte slave address
	//1 byte function code
	//1 byte of data byte count
	//n bytes of data
	return {retVal, replyLength};
}

pair<string, uint16_t> ModbusIP::construct_write_single_holding_register(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, uint16_t value){
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)0;	//frame length
	retVal += (char)PRESET_SINGLE_REGISTER_CODE;	//frame length
	retVal += slaveAddress;
	retVal += (char)6;	//function code
	retVal += (char)(registerAddress>>8);
	retVal += (char)registerAddress;
	retVal += (char)(value>>8);
	retVal += (char)value;
	uint16_t replyLength = retVal.size();
	//reply:
	//Same as request
	return {retVal, replyLength};
}

pair<string, uint16_t> ModbusIP::construct_write_multiple_holding_registers(uint16_t sequenceNumber, uint8_t slaveAddress, uint16_t registerAddress, const std::vector<RegisterValue>& values){
	uint16_t frameLength = 5 + values.size()*2;
	uint16_t registerCount = values.size();
	uint8_t byteCount = registerCount*2;
	string retVal;
	retVal += (char)(sequenceNumber>>8);
	retVal += (char)sequenceNumber;
	retVal += (char)0;
	retVal += (char)0;
	retVal += (char)(frameLength>>8);	//frame length
	retVal += (char)frameLength;	//frame length
	retVal += slaveAddress;
	retVal += (char)PRESET_MULTIPLE_REGISTERS_CODE;	//function code
	retVal += (char)(registerAddress>>8);
	retVal += (char)registerAddress;
	retVal += (char)(registerCount>>8);
	retVal += (char)(registerCount);
	retVal += byteCount;
	for(unsigned i = 0; i<values.size(); ++i){
		retVal += values[i].high;
		retVal += values[i].low;
	}
	//reply:
	//2 bytes sequence number
	//2 bytes protocol number, always 0
	//2 bytes following length
	//1 byte slave address
	//1 byte function code
	//2 bytes of address of first got written register
	//2 bytes of number of register got written
	return {retVal, 12};
}

ModbusIP::ReplyData ModbusIP::decode_reply(const string& reply)
{
	ReplyData retVal;
	retVal.sequenceNumber = reply[0];
	retVal.sequenceNumber<<=8;
	retVal.sequenceNumber += reply[1];
	retVal.slaveAddress = reply[6];
	retVal.functionCode = reply[7];
	switch (retVal.functionCode)
	{
	case READ_COIL_CODE:
	case READ_INPUT_CODE:
	{
		uint8_t bytesCount = reply[8];
		if (bytesCount != (reply.size() - 9))
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
				data.push_back((reply[9+i]>>j)&0x01);
			}
		}
		break;
	}
	case READ_HOLDING_REGISTER_CODE:
	case READ_INPUT_REGISTER_CODE:
	{
		uint8_t bytesCount = reply[8];
		if (bytesCount != (reply.size()-9))
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new vector<RegisterValue>();
		vector<RegisterValue>& data = *((vector<RegisterValue>*) retVal.data);
		for (unsigned i = 0; i < bytesCount; i += 2)
		{
			RegisterValue aValue;
			aValue.high = (uint8_t) reply[9 + i];
			aValue.low = (uint8_t)reply[10 + i];
			data.push_back(aValue);
		}
		break;
	}
	case FORCE_SINGLE_COIL_CODE:
	{
		retVal.data = new pair<uint16_t, bool>();
		pair<uint16_t, bool>& data = *((pair<uint16_t, bool>*) retVal.data);
		data.first = (reply[8] & 0xff);
		data.first <<= 8;
		data.first += (reply[9] & 0xff);
		data.second = (reply[10] == 0xff);
		break;
	}
	case PRESET_SINGLE_REGISTER_CODE:
	{
		if (reply.size() < 12)
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new pair<uint16_t, RegisterValue>();
		pair<uint16_t, RegisterValue>& data = *((pair<uint16_t, RegisterValue>*) retVal.data);
		data.first = (reply[8] & 0xff);
		data.first <<= 8;
		data.first += (reply[9] & 0xff);
		data.second.high = reply[10];
		data.second.low = reply[11];
		break;
	}
	case FORCE_MULTIPLE_COILS_CODE:
	case PRESET_MULTIPLE_REGISTERS_CODE:
	{
		if (reply.size() < 12)
		{
			retVal.functionCode = 0;
			return retVal;
		}
		retVal.data = new pair<uint16_t, uint16_t>();
		pair<uint16_t, uint16_t>& data = *((pair<uint16_t, uint16_t>*) retVal.data);
		data.first = (reply[8] & 0xff);
		data.first <<= 8;
		data.first += (reply[9] & 0xff);
		data.second = (reply[10] & 0xff);
		data.second <<= 8;
		data.second += (reply[11] & 0xff);
		break;
	}
	default:
		retVal.functionCode = 0;
		break;
	}
	return retVal;
}
