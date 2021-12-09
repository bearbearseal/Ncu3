#include "ModbusRegisterValue.h"
#include "../../MyLib/Basic/Sys.h"

using namespace std;

ModbusRegisterValue::ModbusRegisterValue(GlobalEnum::ModbusDataType _type, bool isSmallEndian)
{
	type = _type;
	smallEndian = isSmallEndian;
	switch (type)
	{
	case GlobalEnum::ModbusDataType::UNKNOWN:
	case GlobalEnum::ModbusDataType::COIL:
	case GlobalEnum::ModbusDataType::DIGITAL_INPUT:
	case GlobalEnum::ModbusDataType::INT16:
	case GlobalEnum::ModbusDataType::UINT16:
	case GlobalEnum::ModbusDataType::I_INT16:
	case GlobalEnum::ModbusDataType::I_UINT16:
		registerValues.resize(1);
		break;
	case GlobalEnum::ModbusDataType::INT32_LM:
	case GlobalEnum::ModbusDataType::INT32_ML:
	case GlobalEnum::ModbusDataType::UINT32_LM:
	case GlobalEnum::ModbusDataType::UINT32_ML:
	case GlobalEnum::ModbusDataType::FLOAT32_LM:
	case GlobalEnum::ModbusDataType::FLOAT32_ML:
	case GlobalEnum::ModbusDataType::I_INT32_LM:
	case GlobalEnum::ModbusDataType::I_INT32_ML:
	case GlobalEnum::ModbusDataType::I_UINT32_LM:
	case GlobalEnum::ModbusDataType::I_UINT32_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT32_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT32_ML:
		registerValues.resize(2);
		break;
	case GlobalEnum::ModbusDataType::INT64_LM:
	case GlobalEnum::ModbusDataType::INT64_ML:
	case GlobalEnum::ModbusDataType::UINT64_LM:
	case GlobalEnum::ModbusDataType::UINT64_ML:
	case GlobalEnum::ModbusDataType::FLOAT64_LM:
	case GlobalEnum::ModbusDataType::FLOAT64_ML:
	case GlobalEnum::ModbusDataType::I_INT64_LM:
	case GlobalEnum::ModbusDataType::I_INT64_ML:
	case GlobalEnum::ModbusDataType::I_UINT64_LM:
	case GlobalEnum::ModbusDataType::I_UINT64_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT64_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT64_ML:
		registerValues.resize(4);
		break;
	}
}

ModbusRegisterValue::~ModbusRegisterValue()
{

}

bool ModbusRegisterValue::set_register_value(RegisterValue newValue, unsigned index)
{
	if (index < registerValues.size())
	{
		registerValues[index] = newValue;
		return true;
	}
	return false;
}

bool ModbusRegisterValue::set_register_value(const std::vector<RegisterValue>& newValue, unsigned index)
{
	if(registerValues.size() <= (newValue.size()-index))
	{
		for(unsigned i=0; i<registerValues.size(); ++i)
		{
			registerValues[i] = newValue[i+index];
		}
		return true;
	}
	return false;
}

uint16_t ModbusRegisterValue::get_value(uint8_t high, uint8_t low) const
{
	if (smallEndian)
	{
		return get_value_small_endian(high, low);
	}
	else
	{
		return get_value_big_endian(high, low);
	}
}

uint8_t ModbusRegisterValue::get_register_count(GlobalEnum::ModbusDataType type)
{
	switch (type)
	{
	case GlobalEnum::ModbusDataType::UNKNOWN:	//Should not happen
	case GlobalEnum::ModbusDataType::COIL:	//Should not happen
	case GlobalEnum::ModbusDataType::DIGITAL_INPUT:	//Should not happen
		return 0;
	case GlobalEnum::ModbusDataType::INT16:
	case GlobalEnum::ModbusDataType::UINT16:
	case GlobalEnum::ModbusDataType::I_INT16:
	case GlobalEnum::ModbusDataType::I_UINT16:
		return 1;
	case GlobalEnum::ModbusDataType::INT32_LM:
	case GlobalEnum::ModbusDataType::INT32_ML:
	case GlobalEnum::ModbusDataType::UINT32_LM:
	case GlobalEnum::ModbusDataType::UINT32_ML:
	case GlobalEnum::ModbusDataType::FLOAT32_LM:
	case GlobalEnum::ModbusDataType::FLOAT32_ML:
	case GlobalEnum::ModbusDataType::I_INT32_LM:
	case GlobalEnum::ModbusDataType::I_INT32_ML:
	case GlobalEnum::ModbusDataType::I_UINT32_LM:
	case GlobalEnum::ModbusDataType::I_UINT32_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT32_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT32_ML:
		return 2;
	case GlobalEnum::ModbusDataType::INT64_LM:
	case GlobalEnum::ModbusDataType::INT64_ML:
	case GlobalEnum::ModbusDataType::UINT64_LM:
	case GlobalEnum::ModbusDataType::UINT64_ML:
	case GlobalEnum::ModbusDataType::FLOAT64_LM:
	case GlobalEnum::ModbusDataType::FLOAT64_ML:
	case GlobalEnum::ModbusDataType::I_INT64_LM:
	case GlobalEnum::ModbusDataType::I_INT64_ML:
	case GlobalEnum::ModbusDataType::I_UINT64_LM:
	case GlobalEnum::ModbusDataType::I_UINT64_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT64_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT64_ML:
		return 4;
	}
	return 0;
}

uint16_t ModbusRegisterValue::get_value_small_endian(uint8_t high, uint8_t low) const
{
	uint16_t retVal;
	retVal = high;
	retVal <<= 8;
	retVal += low;
	return retVal;
}

uint16_t ModbusRegisterValue::get_value_big_endian(uint8_t high, uint8_t low) const
{
	uint16_t retVal;
	retVal = low;
	retVal <<= 8;
	retVal += high;
	return retVal;
}

RegisterValue ModbusRegisterValue::get_register(uint16_t singleValue)
{
	if (smallEndian)
	{
		return get_register_small_endian(singleValue);
	}
	else
	{
		return get_register_big_endian(singleValue);
	}
}

RegisterValue ModbusRegisterValue::get_register_small_endian(uint16_t singleValue)
{
	RegisterValue retVal;
	retVal.high = (uint8_t) (singleValue >> 8);
	retVal.low = (uint8_t) singleValue;
	return retVal;
}

RegisterValue ModbusRegisterValue::get_register_big_endian(uint16_t singleValue)
{
	RegisterValue retVal;
	retVal.low = (uint8_t)(singleValue >> 8);
	retVal.high = (uint8_t)singleValue;
	return retVal;
}

Value ModbusRegisterValue::get_value() const
{
	Value retVal;
	switch (type)
	{
	case GlobalEnum::ModbusDataType::COIL:	//Should not happen
	case GlobalEnum::ModbusDataType::DIGITAL_INPUT:
		break;
	case GlobalEnum::ModbusDataType::INT16:
	case GlobalEnum::ModbusDataType::I_INT16:
		retVal.set_integer((int16_t)get_value(registerValues[0].high, registerValues[0].low));
		break;
	case GlobalEnum::ModbusDataType::UINT16:
	case GlobalEnum::ModbusDataType::I_UINT16:
		retVal.set_integer((uint16_t)get_value(registerValues[0].high, registerValues[0].low));
		break;
	case GlobalEnum::ModbusDataType::INT32_LM:
	case GlobalEnum::ModbusDataType::I_INT32_LM:
	case GlobalEnum::ModbusDataType::UINT32_LM:
	case GlobalEnum::ModbusDataType::I_UINT32_LM:
	{
		int32_t result = get_value(registerValues[1].high, registerValues[1].low);
		result <<= 16;
		result |= get_value(registerValues[0].high, registerValues[0].low);
		retVal.set_integer(result);
		break;
	}
	case GlobalEnum::ModbusDataType::INT32_ML:
	case GlobalEnum::ModbusDataType::I_INT32_ML:
	case GlobalEnum::ModbusDataType::UINT32_ML:
	case GlobalEnum::ModbusDataType::I_UINT32_ML:
	{
		int32_t result = get_value(registerValues[0].high, registerValues[0].low);
		result <<= 16;
		result |= get_value(registerValues[1].high, registerValues[1].low);
		retVal.set_integer(result);
		break;
	}
	case GlobalEnum::ModbusDataType::INT64_LM:
	case GlobalEnum::ModbusDataType::UINT64_LM:
	case GlobalEnum::ModbusDataType::I_INT64_LM:
	case GlobalEnum::ModbusDataType::I_UINT64_LM:
	{
		int64_t result = get_value(registerValues[3].high, registerValues[3].low);
		result <<= 16;
		result |= get_value(registerValues[2].high, registerValues[2].low);
		result <<= 16;
		result |= get_value(registerValues[1].high, registerValues[1].low);
		result <<= 16;
		result |= get_value(registerValues[0].high, registerValues[0].low);
		retVal.set_integer(result);
		break;
	}
	case GlobalEnum::ModbusDataType::INT64_ML:
	case GlobalEnum::ModbusDataType::UINT64_ML:
	case GlobalEnum::ModbusDataType::I_INT64_ML:
	case GlobalEnum::ModbusDataType::I_UINT64_ML:
	{
		uint64_t result = get_value(registerValues[0].high, registerValues[0].low);
		result <<= 16;
		result |= get_value(registerValues[1].high, registerValues[1].low);
		result <<= 16;
		result |= get_value(registerValues[2].high, registerValues[2].low);
		result <<= 16;
		result |= get_value(registerValues[3].high, registerValues[3].low);
		retVal.set_integer(result);
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT32_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT32_LM:
	{
		uint8_t result[4];
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[0] = registerValues[0].high;
				result[1] = registerValues[0].low;
				result[2] = registerValues[1].high;
				result[3] = registerValues[1].low;
			}
			else
			{
				//[low][high]
				result[0] = registerValues[0].low;
				result[1] = registerValues[0].high;
				result[2] = registerValues[1].low;
				result[3] = registerValues[1].high;
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[3] = registerValues[0].high;
				result[2] = registerValues[0].low;
				result[1] = registerValues[1].high;
				result[0] = registerValues[1].low;
			}
			else
			{
				//[low][high]
				result[3] = registerValues[0].low;
				result[2] = registerValues[0].high;
				result[1] = registerValues[1].low;
				result[0] = registerValues[1].high;
			}
		}
		retVal.set_float(*((float*)&result));
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT32_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT32_ML:
	{
		uint8_t result[4];
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[0] = registerValues[1].high;
				result[1] = registerValues[1].low;
				result[2] = registerValues[0].high;
				result[3] = registerValues[0].low;
			}
			else
			{
				//[low][high]
				result[0] = registerValues[1].low;
				result[1] = registerValues[1].high;
				result[2] = registerValues[0].low;
				result[3] = registerValues[0].high;
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[3] = registerValues[1].high;
				result[2] = registerValues[1].low;
				result[1] = registerValues[0].high;
				result[0] = registerValues[0].low;
			}
			else
			{
				//[low][high]
				result[3] = registerValues[1].low;
				result[2] = registerValues[1].high;
				result[1] = registerValues[0].low;
				result[0] = registerValues[0].high;
			}
		}
		retVal.set_float(*((float*)&result));
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT64_LM:
	case GlobalEnum::ModbusDataType::I_FLOAT64_LM:
	{
		uint8_t result[8];
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[0] = registerValues[0].high;
				result[1] = registerValues[0].low;
				result[2] = registerValues[1].high;
				result[3] = registerValues[1].low;
				result[4] = registerValues[2].high;
				result[5] = registerValues[2].low;
				result[6] = registerValues[3].high;
				result[7] = registerValues[3].low;
			}
			else
			{
				//[low][high]
				result[0] = registerValues[0].low;
				result[1] = registerValues[0].high;
				result[2] = registerValues[1].low;
				result[3] = registerValues[1].high;
				result[4] = registerValues[2].low;
				result[5] = registerValues[2].high;
				result[6] = registerValues[3].low;
				result[7] = registerValues[3].high;
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[7] = registerValues[0].high;
				result[6] = registerValues[0].low;
				result[5] = registerValues[1].high;
				result[4] = registerValues[1].low;
				result[3] = registerValues[2].high;
				result[2] = registerValues[2].low;
				result[1] = registerValues[3].high;
				result[0] = registerValues[3].low;
			}
			else
			{
				//[low][high]
				result[7] = registerValues[0].low;
				result[6] = registerValues[0].high;
				result[5] = registerValues[1].low;
				result[4] = registerValues[1].high;
				result[3] = registerValues[2].low;
				result[2] = registerValues[2].high;
				result[1] = registerValues[3].low;
				result[0] = registerValues[3].high;
			}
		}
		retVal.set_float(*((float*)&result));
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT64_ML:
	case GlobalEnum::ModbusDataType::I_FLOAT64_ML:
	{
		uint8_t result[8];
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[0] = registerValues[3].high;
				result[1] = registerValues[3].low;
				result[2] = registerValues[2].high;
				result[3] = registerValues[2].low;
				result[4] = registerValues[1].high;
				result[5] = registerValues[1].low;
				result[6] = registerValues[0].high;
				result[7] = registerValues[0].low;
			}
			else
			{
				//[low][high]
				result[0] = registerValues[3].low;
				result[1] = registerValues[3].high;
				result[2] = registerValues[2].low;
				result[3] = registerValues[2].high;
				result[4] = registerValues[1].low;
				result[5] = registerValues[1].high;
				result[6] = registerValues[0].low;
				result[7] = registerValues[0].high;
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				result[7] = registerValues[3].high;
				result[6] = registerValues[3].low;
				result[5] = registerValues[2].high;
				result[4] = registerValues[2].low;
				result[3] = registerValues[1].high;
				result[2] = registerValues[1].low;
				result[1] = registerValues[0].high;
				result[0] = registerValues[0].low;
			}
			else
			{
				//[low][high]
				result[7] = registerValues[3].low;
				result[6] = registerValues[3].high;
				result[5] = registerValues[2].low;
				result[4] = registerValues[2].high;
				result[3] = registerValues[1].low;
				result[2] = registerValues[1].high;
				result[1] = registerValues[0].low;
				result[0] = registerValues[0].high;
			}
		}
		retVal.set_float(*((float*)&result));
		break;
	}
	default:
		break;
	}
	return retVal;
}

std::vector<RegisterValue> ModbusRegisterValue::convert_to_register_value(const Value& rawValue)
{
	vector<RegisterValue> retVal;
	switch (type)
	{
	case GlobalEnum::ModbusDataType::COIL:
		break;
	case GlobalEnum::ModbusDataType::INT16:
	{
		int16_t int16 = (int16_t) rawValue.get_int();
		RegisterValue aValue = get_register(int16);
		retVal.push_back(aValue);
		break;
	}
	case GlobalEnum::ModbusDataType::UINT16:
	{
		uint16_t uint16 = (uint16_t)rawValue.get_int();
		RegisterValue aValue = get_register(uint16);
		retVal.push_back(aValue);
		break;
	}
	case GlobalEnum::ModbusDataType::INT32_LM:
	{
		int32_t int32 = (int32_t)rawValue.get_int();
		int16_t int16 = (int16_t) int32;
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int32 >> 16);
		retVal.push_back(get_register(int16));
		break;
	}
	case GlobalEnum::ModbusDataType::INT32_ML:
	{
		int32_t int32 = (int32_t)rawValue.get_int();
		int16_t int16 = (int16_t)(int32 >> 16);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)int32;
		retVal.push_back(get_register(int16));
		break;
	}
	case GlobalEnum::ModbusDataType::UINT32_LM:
	{
		uint32_t uint32 = (uint32_t)rawValue.get_int();
		uint16_t uint16 = (uint16_t)uint32;
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint32 >> 16);
		retVal.push_back(get_register(uint16));
		break;
	}
	case GlobalEnum::ModbusDataType::UINT32_ML:
	{
		int32_t int32 = (int32_t)rawValue.get_int();
		int16_t int16 = (int16_t)(int32 >> 16);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)int32;
		retVal.push_back(get_register(int16));
		break;
	}
	case GlobalEnum::ModbusDataType::INT64_LM:
	{
		int64_t int64 = (int64_t)rawValue.get_int();
		int16_t int16 = (int16_t)int64;
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int64 >> 16);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int64 >> 32);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int64 >> 48);
		retVal.push_back(get_register(int16));
		break;
	}
	case GlobalEnum::ModbusDataType::INT64_ML:
	{
		int64_t int64 = (int64_t)rawValue.get_int();
		int16_t int16 = (int16_t)(int64 >> 48);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int64 >> 32);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)(int64 >> 16);
		retVal.push_back(get_register(int16));
		int16 = (int16_t)int64;
		retVal.push_back(get_register(int16));
		break;
	}
	case GlobalEnum::ModbusDataType::UINT64_LM:
	{
		uint64_t uint64 = (uint64_t)rawValue.get_int();
		uint16_t uint16 = (uint16_t)uint64;
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint64 >> 16);
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint64 >> 32);
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint64 >> 48);
		retVal.push_back(get_register(uint16));
		break;
	}
	case GlobalEnum::ModbusDataType::UINT64_ML:
	{
		uint64_t uint64 = (uint64_t)rawValue.get_int();
		uint16_t uint16 = (uint16_t)(uint64 >> 48);
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint64 >> 32);
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)(uint64 >> 16);
		retVal.push_back(get_register(uint16));
		uint16 = (uint16_t)uint64;
		retVal.push_back(get_register(uint16));
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT32_LM:
	{
		float theFloat = (float)rawValue.get_float();
		uint8_t* result = ((uint8_t*) &theFloat);
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[0];
				registerValue.low = result[1];
				retVal.push_back(registerValue);
				registerValue.high = result[2];
				registerValue.low = result[3];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[0];
				registerValue.high = result[1];
				retVal.push_back(registerValue);
				registerValue.low = result[2];
				registerValue.high = result[3];
				retVal.push_back(registerValue);
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[3];
				registerValue.low = result[2];
				retVal.push_back(registerValue);
				registerValue.high = result[1];
				registerValue.low = result[0];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[3];
				registerValue.high = result[2];
				retVal.push_back(registerValue);
				registerValue.low = result[1];
				registerValue.high = result[0];
				retVal.push_back(registerValue);
			}
		}
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT32_ML:
	{
		float theFloat = (float)rawValue.get_float();
		uint8_t* result = ((uint8_t*)&theFloat);
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[2];
				registerValue.low = result[3];
				retVal.push_back(registerValue);
				registerValue.high = result[0];
				registerValue.low = result[1];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[2];
				registerValue.high = result[3];
				retVal.push_back(registerValue);
				registerValue.low = result[0];
				registerValue.high = result[1];
				retVal.push_back(registerValue);
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[1];
				registerValue.low = result[0];
				retVal.push_back(registerValue);
				registerValue.high = result[3];
				registerValue.low = result[2];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[1];
				registerValue.high = result[0];
				retVal.push_back(registerValue);
				registerValue.low = result[3];
				registerValue.high = result[2];
				retVal.push_back(registerValue);
			}
		}
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT64_LM:
	{
		double theFloat = (double)rawValue.get_float();
		uint8_t* result = ((uint8_t*)&theFloat);
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[0];
				registerValue.low = result[1];
				retVal.push_back(registerValue);
				registerValue.high = result[2];
				registerValue.low = result[3];
				retVal.push_back(registerValue);
				registerValue.high = result[4];
				registerValue.low = result[5];
				retVal.push_back(registerValue);
				registerValue.high = result[6];
				registerValue.low = result[7];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[0];
				registerValue.high = result[1];
				retVal.push_back(registerValue);
				registerValue.low = result[2];
				registerValue.high = result[3];
				retVal.push_back(registerValue);
				registerValue.low = result[4];
				registerValue.high = result[5];
				retVal.push_back(registerValue);
				registerValue.low = result[6];
				registerValue.high = result[7];
				retVal.push_back(registerValue);
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[7];
				registerValue.low = result[6];
				retVal.push_back(registerValue);
				registerValue.high = result[5];
				registerValue.low = result[4];
				retVal.push_back(registerValue);
				registerValue.high = result[3];
				registerValue.low = result[2];
				retVal.push_back(registerValue);
				registerValue.high = result[1];
				registerValue.low = result[0];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[7];
				registerValue.high = result[6];
				retVal.push_back(registerValue);
				registerValue.low = result[5];
				registerValue.high = result[4];
				retVal.push_back(registerValue);
				registerValue.low = result[3];
				registerValue.high = result[2];
				retVal.push_back(registerValue);
				registerValue.low = result[1];
				registerValue.high = result[0];
				retVal.push_back(registerValue);
			}
		}
		break;
	}
	case GlobalEnum::ModbusDataType::FLOAT64_ML:
	{
		double theFloat = (double)rawValue.get_float();
		uint8_t* result = ((uint8_t*)&theFloat);
		//result[0] is the most significant
		if (Sys::little_endian())
		{
			//result[0] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[6];
				registerValue.low = result[7];
				retVal.push_back(registerValue);
				registerValue.high = result[4];
				registerValue.low = result[5];
				retVal.push_back(registerValue);
				registerValue.high = result[2];
				registerValue.low = result[3];
				retVal.push_back(registerValue);
				registerValue.high = result[0];
				registerValue.low = result[1];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[6];
				registerValue.high = result[7];
				retVal.push_back(registerValue);
				registerValue.low = result[4];
				registerValue.high = result[5];
				retVal.push_back(registerValue);
				registerValue.low = result[2];
				registerValue.high = result[3];
				retVal.push_back(registerValue);
				registerValue.low = result[0];
				registerValue.high = result[1];
				retVal.push_back(registerValue);
			}
		}
		//result[3] is the most significant
		else
		{
			//result[3] shall equals registerValues[0].high
			if (smallEndian)
			{
				//[high][low]
				RegisterValue registerValue;
				registerValue.high = result[1];
				registerValue.low = result[0];
				retVal.push_back(registerValue);
				registerValue.high = result[3];
				registerValue.low = result[2];
				retVal.push_back(registerValue);
				registerValue.high = result[5];
				registerValue.low = result[4];
				retVal.push_back(registerValue);
				registerValue.high = result[7];
				registerValue.low = result[6];
				retVal.push_back(registerValue);
			}
			else
			{
				//[low][high]
				RegisterValue registerValue;
				registerValue.low = result[1];
				registerValue.high = result[0];
				retVal.push_back(registerValue);
				registerValue.low = result[3];
				registerValue.high = result[2];
				retVal.push_back(registerValue);
				registerValue.low = result[5];
				registerValue.high = result[4];
				retVal.push_back(registerValue);
				registerValue.low = result[7];
				registerValue.high = result[6];
				retVal.push_back(registerValue);
			}
		}
		break;
	}
	default:
		break;
	}
	return retVal;
}
