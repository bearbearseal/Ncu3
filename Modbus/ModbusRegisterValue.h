#ifndef _MODBUSREGISTERVALUE_H_
#define _MODBUSREGISTERVALUE_H_
#include "../../MyLib/Basic/Value.h"
#include "ModbusDefinition.h"
#include <vector>
#include <tuple>

class ModbusRegisterValue
{
public:
	enum class DataType
	{
		UNKNOWN = 0,
		COIL = 1,
		INT16 = 11,
		INT32_LM = 12,
		INT32_ML = 13,
		INT64_LM = 14,
		INT64_ML = 15,
		UINT16 = 21,
		UINT32_LM = 22,
		UINT32_ML = 23,
		UINT64_LM = 24,
		UINT64_ML = 25,
		FLOAT32_LM = 31,
		FLOAT32_ML = 32,
		FLOAT64_LM = 33,
		FLOAT64_ML = 34
	};

	ModbusRegisterValue(DataType _type, bool smallEndian = true);
	~ModbusRegisterValue();

	bool set_register_value(RegisterValue newValue, unsigned index = 0);
	bool set_register_value(const std::vector<RegisterValue>& newValue, unsigned index=0);
	std::vector<RegisterValue> convert_to_register_value(const Value& rawValue);
	Value get_value() const;
	static DataType convert_integer_to_data_type(uint16_t type);
	static uint8_t get_register_count(DataType type);

private:
	uint16_t get_value(uint8_t high, uint8_t low) const;
	uint16_t get_value_small_endian(uint8_t high, uint8_t low) const;
	uint16_t get_value_big_endian(uint8_t high, uint8_t low) const;

	RegisterValue get_register(uint16_t singleValue);
	RegisterValue get_register_small_endian(uint16_t singleValue);
	RegisterValue get_register_big_endian(uint16_t singleValue);

	DataType type;
	bool smallEndian;	//modbus is small endian by default
	std::vector<RegisterValue> registerValues;
};
#endif
