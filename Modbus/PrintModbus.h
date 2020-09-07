#ifndef _PRINTMODBUS_H_
#define _PRINTMODBUS_H_
#include <stdio.h>
#include "ModbusRTU.h"

namespace PrintModbus
{
	void print_value_in_hex(const std::string& source);

	void print_modbus_reply(const ModbusRtu::ReplyData& data);
}

#endif
