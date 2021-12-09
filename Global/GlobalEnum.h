#ifndef _GlobalEnum_H_
#define _GlobalEnum_H_
#include <stdint.h>

namespace GlobalEnum {
    enum class Compare : uint8_t
    {
        GREATER         = 1,
        GREATEREQUAL    = 2,
        EQUAL           = 3,
        SMALLEREQUAL    = 4,
        SMALLER         = 5,
        NOTEQUAL        = 6,
        INVALID
    };
    bool is_compare(uint8_t value);

    enum class ScheduleAction : uint8_t {
        SET     = 1,
        UNSET   = 2,
        WRITE   = 3,
        INVALID
    };
    bool is_schedule_action(uint8_t action);

    enum class ScheduleSubject : uint8_t
    {
        WEEK_DAY        = 1, //Sunday is 0, Monday is 1
        MONTH           = 2,
        MONTH_DAY       = 3,
        MONTH_WEEK      = 4, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        MONTH_SUNDAY    = 5,
        MONTH_MONDAY    = 6,
        MONTH_TUESDAY   = 7,
        MONTH_WEDNESDAY = 8,
        MONTH_THURSDAY  = 9,
        MONTH_FRIDAY    = 10,
        MONTH_SATURDAY  = 11,
        YEAR            = 12,
        YEAR_DAY        = 13, //1st day is 1 instead of 0
        YEAR_WEEK       = 14, //1st week is 1 instead of 0, weeks with < 7 days is counted.
        DDMM            = 15,
        DDMMYYYY        = 16,
        INVALID
    };
    bool is_schedule_subject(uint8_t subject);

    enum class ModbusDataType : uint8_t
	{
		UNKNOWN = 0,
		COIL = 1,
		DIGITAL_INPUT = 2,
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
		FLOAT64_ML = 34,
		//Input Register
		I_INT16 = 111,
		I_INT32_LM = 112,
		I_INT32_ML = 113,
		I_INT64_LM = 114,
		I_INT64_ML = 115,
		I_UINT16 = 121,
		I_UINT32_LM = 122,
		I_UINT32_ML = 123,
		I_UINT64_LM = 124,
		I_UINT64_ML = 125,
		I_FLOAT32_LM = 131,
		I_FLOAT32_ML = 132,
		I_FLOAT64_LM = 133,
		I_FLOAT64_ML = 134
	};
    bool is_modbus_data_type(uint8_t value);
    bool is_modbus_coil_type(uint8_t value);
    bool is_modbus_coil_type(ModbusDataType value);
    bool is_modbus_digital_input_type(uint8_t value);
    bool is_modbus_digital_input_type(ModbusDataType value);
    bool is_modbus_holding_register_type(uint8_t value);
    bool is_modbus_holding_register_type(ModbusDataType value);
    bool is_modbus_input_register_type(uint8_t value);
    bool is_modbus_input_register_type(ModbusDataType value);
}

#endif