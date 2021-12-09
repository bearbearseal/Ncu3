#ifndef _MODBUSREGISTERVALUE_H_
#define _MODBUSREGISTERVALUE_H_
#include "../../MyLib/Basic/Value.h"
#include "ModbusDefinition.h"
#include "../Global/GlobalEnum.h"
#include <vector>
#include <tuple>

class ModbusRegisterValue
{
public:
	//Need definition of Discrete input and Input register
	ModbusRegisterValue(GlobalEnum::ModbusDataType _type, bool smallEndian = true);
	~ModbusRegisterValue();

	bool set_register_value(RegisterValue newValue, unsigned index = 0);
	bool set_register_value(const std::vector<RegisterValue>& newValue, unsigned index=0);
	std::vector<RegisterValue> convert_to_register_value(const Value& rawValue);
	Value get_value() const;
	static uint8_t get_register_count(GlobalEnum::ModbusDataType type);

private:
	uint16_t get_value(uint8_t high, uint8_t low) const;
	uint16_t get_value_small_endian(uint8_t high, uint8_t low) const;
	uint16_t get_value_big_endian(uint8_t high, uint8_t low) const;

	RegisterValue get_register(uint16_t singleValue);
	RegisterValue get_register_small_endian(uint16_t singleValue);
	RegisterValue get_register_big_endian(uint16_t singleValue);

	GlobalEnum::ModbusDataType type;
	bool smallEndian;	//modbus is small endian by default
	std::vector<RegisterValue> registerValues;
};
#endif
