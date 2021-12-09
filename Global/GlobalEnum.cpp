#include "GlobalEnum.h"

using namespace std;

bool GlobalEnum::is_compare(uint8_t value)
{
    return value >= static_cast<uint8_t>(Compare::GREATER) && value <= static_cast<uint8_t>(Compare::NOTEQUAL);
}

bool GlobalEnum::is_schedule_action(uint8_t action)
{
    return action >= static_cast<uint8_t>(ScheduleAction::SET) && action <= static_cast<uint8_t>(ScheduleAction::WRITE);
}

bool GlobalEnum::is_schedule_subject(uint8_t subject)
{
    return (subject >= int(GlobalEnum::ScheduleSubject::WEEK_DAY) && subject <= int(GlobalEnum::ScheduleSubject::DDMMYYYY));
}

bool GlobalEnum::is_modbus_data_type(uint8_t value)
{
    return value >= static_cast<uint8_t>(ModbusDataType::COIL) && value <= static_cast<uint8_t>(ModbusDataType::I_FLOAT64_ML);
}

bool GlobalEnum::is_modbus_coil_type(uint8_t value)
{
    return value == static_cast<uint8_t>(ModbusDataType::COIL);
}

bool GlobalEnum::is_modbus_coil_type(ModbusDataType value)
{
    return value == ModbusDataType::COIL;
}

bool GlobalEnum::is_modbus_digital_input_type(uint8_t value)
{
    return value == static_cast<uint8_t>(ModbusDataType::DIGITAL_INPUT);
}

bool GlobalEnum::is_modbus_digital_input_type(ModbusDataType value)
{
    return value == ModbusDataType::DIGITAL_INPUT;
}

bool GlobalEnum::is_modbus_holding_register_type(uint8_t value)
{
    return value >= static_cast<uint8_t>(ModbusDataType::INT16) && value <= static_cast<uint8_t>(ModbusDataType::FLOAT64_ML);
}

bool GlobalEnum::is_modbus_holding_register_type(ModbusDataType value)
{
    return value >= ModbusDataType::INT16 && value <= ModbusDataType::FLOAT64_ML;
}

bool GlobalEnum::is_modbus_input_register_type(uint8_t value)
{
    return value >= static_cast<uint8_t>(ModbusDataType::I_INT16) && value <= static_cast<uint8_t>(ModbusDataType::I_FLOAT64_ML);
}

bool GlobalEnum::is_modbus_input_register_type(ModbusDataType value)
{
    return value >= ModbusDataType::I_INT16 && value <= ModbusDataType::I_FLOAT64_ML;
}
