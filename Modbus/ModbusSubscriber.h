#ifndef _MODBUSSUBSCRIBER_H_
#define _MODBUSSUBSCRIBER_H_
#include <stdint.h>
#include <vector>
#include "ModbusDefinition.h"
#include "../Basic/Value.h"

namespace ModbusSubscriber
{
	class HoldingRegister
	{
	public:
		virtual ~HoldingRegister(){}
		virtual void catch_holding_register_value_change(const Value& values) = 0;
	};
	class InputRegister
	{
	public:
		virtual ~InputRegister(){}
		virtual void catch_input_register_value_change(uint8_t slaveAddress, uint16_t registerAddress, uint8_t high, uint8_t low) = 0;
	};
	class CoilStatus
	{
	public:
		virtual ~CoilStatus(){}
		virtual void catch_coil_status_value_change(bool value) = 0;
	};
	class InputStatus
	{
	public:
		virtual ~InputStatus(){}
		virtual void catch_input_status_value_change(uint8_t slaveAddress, uint16_t inputAddress, bool value) = 0;
	};
};
#endif
