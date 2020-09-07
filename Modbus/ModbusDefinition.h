#ifndef _MODBUSDEFINITION_H_
#define _MODBUSDEFINITION_H_
#include <stdint.h>

struct RegisterValue
{
	uint8_t high = 0;
	uint8_t low = 0;

	bool operator==(const RegisterValue& theOther)
	{
		return (high == theOther.high && low == theOther.low);
	}
};

#endif
