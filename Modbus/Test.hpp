#ifndef _TEST_HPP_
#define _TEST_HPP_
#include "../MyLib/SerialPort/SerialPort.h"
#include "../MyLib/TcpSocket/TcpSocket.h"
#include "ModbusIP.h"
#include "ModbusIpProcess.h"
#include "ModbusRegisterValue.h"
//#include "ModbusRTU.h"
#include "ModbusRtuProcess.h"
//#include "ModbusRTUChannel.h"
//#include "PrintModbus.h"
#include <thread>

using namespace std;
/*
void test_channel()
{
    //Serial Port
	SerialPort serialPort;
	serialPort.open("/dev/ttyUSB0");
	SerialPort::Config serialConfig;
	serialConfig.parityBit = false;
	serialConfig.stopBit = false;
	serialConfig.bitPerByte = 8;
	serialConfig.hardwareFlowControl = false;
	serialConfig.softwareFlowControl = false;
	serialConfig.blocking = false;
	serialConfig.baudrate = B19200;
	serialPort.configure(serialConfig);
	//End of Serial Port

    //Modbus Process
	//ModbusRTUProcess ModbusRTUProcess(serialPort);
    ModbusRTUChannel modbusRtuChannel(serialPort);
    modbusRtuChannel.add_coil_variable(1, 1, 1);
    modbusRtuChannel.add_coil_variable(2, 1, 34);
    modbusRtuChannel.add_coil_variable(3, 1, 43);
    modbusRtuChannel.add_holding_register_variable(4, 1, 0, ModbusRegisterValue::INT32_ML);
    modbusRtuChannel.add_holding_register_variable(5, 1, 20, ModbusRegisterValue::INT32_LM);
    modbusRtuChannel.add_holding_register_variable(6, 1, 12, ModbusRegisterValue::INT64_ML);
    modbusRtuChannel.add_holding_register_variable(7, 1, 4, ModbusRegisterValue::INT32_LM);
    modbusRtuChannel.add_holding_register_variable(8, 1, 6, ModbusRegisterValue::INT32_ML);
    modbusRtuChannel.add_holding_register_variable(9, 1, 8, ModbusRegisterValue::INT32_ML);
    
    auto variable1 = modbusRtuChannel.get_variable(1);
    auto variable2 = modbusRtuChannel.get_variable(2);
    auto variable3 = modbusRtuChannel.get_variable(3);
    auto variable4 = modbusRtuChannel.get_variable(4);
    auto variable5 = modbusRtuChannel.get_variable(5);
    auto variable6 = modbusRtuChannel.get_variable(6);
    auto variable7 = modbusRtuChannel.get_variable(7);
    auto variable8 = modbusRtuChannel.get_variable(8);
    auto variable9 = modbusRtuChannel.get_variable(9);

    if(variable1 == nullptr)
    {
        printf("Variable is null.\n");
    }
    modbusRtuChannel.start();
    unsigned writeValue = 170;
	bool coilValue = false;
    printf("Starting.\n");
	while (1)
	{
		this_thread::sleep_for(10s);
		printf("Coil 1: %d\n", (int16_t)variable1->get_value().get_int());
		printf("Coil 34: %d\n", (int16_t)variable2->get_value().get_int());
		printf("Coil 43: %d\n", (int16_t)variable3->get_value().get_int());
		printf("Register 0 INT32_ML: %llx\n", variable4->get_value().get_int());
		printf("Register 20 INT32_LM: %llx\n", variable5->get_value().get_int());
		printf("Register 12 INT32_LM: %llx\n", variable6->get_value().get_int());
		printf("Register 4 INT32_LM: %llx\n", variable7->get_value().get_int());
		printf("Register 6 INT32_LM: %llx\n", variable8->get_value().get_int());
		printf("Register 8 INT64_LM: %llx\n", variable9->get_value().get_int());
		variable4->set_value(writeValue);
		variable1->set_value(coilValue);
		++writeValue;
		coilValue = !coilValue;
	}
	printf("End.\n");
}

void test_register()
{
	ModbusRegisterValue int16(ModbusRegisterValue::INT16);
	ModbusRegisterValue uint16(ModbusRegisterValue::UINT16);
	ModbusRegisterValue int32Lm(ModbusRegisterValue::INT32_LM);
	ModbusRegisterValue int32Ml(ModbusRegisterValue::INT32_ML);
	ModbusRegisterValue uint32Lm(ModbusRegisterValue::UINT32_LM);
	ModbusRegisterValue uint32Ml(ModbusRegisterValue::UINT32_ML);
	ModbusRegisterValue int64Lm(ModbusRegisterValue::INT64_LM);
	ModbusRegisterValue int64Ml(ModbusRegisterValue::INT64_ML);
	ModbusRegisterValue uint64Lm(ModbusRegisterValue::UINT64_LM);
	ModbusRegisterValue uint64Ml(ModbusRegisterValue::UINT64_ML);

	ModbusRegisterValue int16_i(ModbusRegisterValue::INT16, false);
	ModbusRegisterValue uint16_i(ModbusRegisterValue::UINT16, false);
	ModbusRegisterValue int32Lm_i(ModbusRegisterValue::INT32_LM, false);
	ModbusRegisterValue int32Ml_i(ModbusRegisterValue::INT32_ML, false);
	ModbusRegisterValue uint32Lm_i(ModbusRegisterValue::UINT32_LM, false);
	ModbusRegisterValue uint32Ml_i(ModbusRegisterValue::UINT32_ML, false);
	ModbusRegisterValue int64Lm_i(ModbusRegisterValue::INT64_LM, false);
	ModbusRegisterValue int64Ml_i(ModbusRegisterValue::INT64_ML, false);
	ModbusRegisterValue uint64Lm_i(ModbusRegisterValue::UINT64_LM, false);
	ModbusRegisterValue uint64Ml_i(ModbusRegisterValue::UINT64_ML, false);

	RegisterValue register1;
	register1.high = (uint8_t)0x80;
	register1.low = (uint8_t)0x08;
	RegisterValue register2;
	register2.high = (uint8_t)0x08;
	register2.low = (uint8_t)0x80;
	RegisterValue register3;
	register3.high = (uint8_t)0xf0;
	register3.low = (uint8_t)0x0f;
	RegisterValue register4;
	register4.high = (uint8_t)0x0f;
	register4.low = (uint8_t)0xf0;
		
	int16.set_register_value(register1);
	uint16.set_register_value(register2);

	printf("int16 %x\n", int16.get_value().get_int());
	printf("uint16 %x\n", uint16.get_value().get_int());

	vector<RegisterValue> array32;// = {register1, register2};
	array32.push_back(register1);
	array32.push_back(register2);

	uint32Lm.set_register_value(array32);
	uint32Ml.set_register_value(array32);
	int32Lm.set_register_value(array32);
	int32Ml.set_register_value(array32);

	printf("uint32LM %llx\n", uint32Lm.get_value().get_int());
	printf("uint32ML %llx\n", uint32Ml.get_value().get_int());
	printf("int32LM %llx\n", int32Lm.get_value().get_int());
	printf("int32ML %llx\n", int32Ml.get_value().get_int());

	vector<RegisterValue> array64;// = { register1, register2, register3, register4 };
	array64.push_back(register1);
	array64.push_back(register2);
	array64.push_back(register3);
	array64.push_back(register4);

	uint64Lm.set_register_value(array64);
	uint64Ml.set_register_value(array64);
	int64Lm.set_register_value(array64);
	int64Ml.set_register_value(array64);

	printf("uint64LM %llx\n", uint64Lm.get_value().get_int());
	printf("uint64ML %llx\n", uint64Ml.get_value().get_int());
	printf("int64LM %llx\n", int64Lm.get_value().get_int());
	printf("int64ML %llx\n", int64Ml.get_value().get_int());
}
*/
void test_modbusip(){
	TcpSocket tcpSocket(true);
    tcpSocket.open("192.168.137.1", 502);
	while(!tcpSocket.connection_established())
	{
		printf("Trying to connect\n");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	printf("Connection established his address: %s:%d.\n", tcpSocket.get_his_ip().c_str(), tcpSocket.get_his_port());

	{
		auto result = ModbusIP::construct_read_coils(1, 1, 10, 12);
		tcpSocket.write(result.first);
		this_thread::sleep_for(100ms);
		auto reply = tcpSocket.read(true);
		if(!reply.first){
			printf("Server does not reply to read coils.\n");
			exit(0);
		}
		if(reply.second.size() < result.second){
			printf("Server read coil reply size not correct.\n");
			exit(0);
		}
		auto data = ModbusIP::decode_reply(reply.second);
		if(data.functionCode != ModbusIP::READ_COIL_CODE){
			printf("Server reply function code not matched %d:%d.\n", data.functionCode, ModbusIP::READ_COIL_CODE);
			exit(0);
		}
		auto coils = data.get_coils();
		printf("Result: ");
		for(auto i = coils.begin(); i != coils.end(); ++i){
			printf("[%s]", *i ? "true": "false");
		}
		printf("\n");
	}
/*
	{
		auto result = ModbusIP::construct_read_multiple_holding_registers(2, 1, 10, 7);
		tcpSocket.write(result.first);
		this_thread::sleep_for(100ms);
		auto reply = tcpSocket.read(true);
		if(!reply.first){
			printf("Server does not reply to read holding registers.\n");
			exit(0);
		}
		if(reply.second.size() < result.second){
			printf("Server read holding registers reply size not correct, expect %d, got %lu.\n", result.second, reply.second.size());
			exit(0);
		}
		auto data = ModbusIP::decode_reply(reply.second);
		if(data.functionCode != ModbusIP::READ_HOLDING_REGISTER_CODE){
			printf("Server reply function code not matched %d:%d.\n", data.functionCode, ModbusIP::READ_HOLDING_REGISTER_CODE);
			exit(0);
		}
		auto registers = data.get_holding_register();
		printf("Result: ");
		for(auto i= registers.begin(); i != registers.end(); ++i){
			printf("[%02X][%02X]", i->high, i->low);
		}
		printf("\n");
	}

	{
		auto result = ModbusIP::construct_read_input_registers(3, 1, 20, 4);
		tcpSocket.write(result.first);
		this_thread::sleep_for(100ms);
		auto reply = tcpSocket.read(true);
		if(!reply.first){
			printf("Server does not reply to read input registers.\n");
			exit(0);
		}
		if(reply.second.size() < result.second){
			printf("Server read input registers reply size not correct, expect %llu, got %llu.\n", result.second, reply.second.size());
			exit(0);
		}
		auto data = ModbusIP::decode_reply(reply.second);
		if(data.functionCode != ModbusIP::READ_INPUT_REGISTER_CODE){
			printf("Server reply function code not matched %u:%u.\n", data.functionCode, ModbusIP::READ_INPUT_REGISTER_CODE);
			exit(0);
		}
		auto registers = data.get_input_register();
		printf("Result: ");
		for(auto i= registers.begin(); i != registers.end(); ++i){
			printf("[%02X][%02X]", i->high, i->low);
		}
		printf("\n");
	}

	{
		auto result = ModbusIP::construct_write_single_coil(4, 1, 10, true);
		tcpSocket.write(result.first);
		this_thread::sleep_for(100ms);
		auto reply = tcpSocket.read(true);
		if(!reply.first){
			printf("Server does not reply to write single coil.\n");
			exit(0);
		}
		if(reply.second.size() < result.second){
			printf("Server write single coil reply size not correct.\n");
			exit(0);
		}
		auto data = ModbusIP::decode_reply(reply.second);
		if(data.functionCode != ModbusIP::FORCE_SINGLE_COIL_CODE){
			printf("Server reply function code not matched %llu:%llu.\n", data.functionCode, ModbusIP::FORCE_SINGLE_COIL_CODE);
			exit(0);
		}
		auto coil = data.get_force_single_coil();
		printf("Result: %u %u\n", coil.first, coil.second);
	}

	{
		auto result = ModbusIP::construct_write_multiple_holding_registers(5, 1, 20, {{0,255}, {1,0}, {1,255}, {2, 0}});
		tcpSocket.write(result.first);
		this_thread::sleep_for(100ms);
		auto reply = tcpSocket.read(true);
		if(!reply.first){
			printf("Server does not reply to write multiple holding registers.\n");
			exit(0);
		}
		if(reply.second.size() < result.second){
			printf("Server write multiple holding register reply size not correct.\n");
			exit(0);
		}
		auto data = ModbusIP::decode_reply(reply.second);
		if(data.functionCode != ModbusIP::PRESET_MULTIPLE_REGISTERS_CODE){
			printf("Server reply function code not matched %u:%u.\n", data.functionCode, ModbusIP::PRESET_MULTIPLE_REGISTERS_CODE);
			exit(0);
		}
		auto writeRegisters = data.get_preset_multiple_registers();
		printf("Result: %u %u\n", writeRegisters.first, writeRegisters.second);
	}
*/
}

void test_modbus_ip_process() {
	ModbusIpProcess modbusIpProcess("192.168.137.1", 502, 1, 16, 64, true, chrono::milliseconds(500));
	auto coil1 = modbusIpProcess.get_coil_status_variable(1);
	auto coil2 = modbusIpProcess.get_coil_status_variable(2);
	auto coil5 = modbusIpProcess.get_coil_status_variable(5);
	auto coil7 = modbusIpProcess.get_coil_status_variable(7);
	auto coil19 = modbusIpProcess.get_coil_status_variable(19);
	auto coil30 = modbusIpProcess.get_coil_status_variable(30);
	auto coil48 = modbusIpProcess.get_coil_status_variable(48);

	auto register4 = modbusIpProcess.get_holding_register_variable(4, ModbusRegisterValue::DataType::INT32_LM);
	auto register8 = modbusIpProcess.get_holding_register_variable(8, ModbusRegisterValue::DataType::INT16);
	auto register9 = modbusIpProcess.get_holding_register_variable(9, ModbusRegisterValue::DataType::INT32_LM);
	auto register12 = modbusIpProcess.get_holding_register_variable(12, ModbusRegisterValue::DataType::INT32_LM);
	auto register14 = modbusIpProcess.get_holding_register_variable(14, ModbusRegisterValue::DataType::INT64_LM);
	auto register18 = modbusIpProcess.get_holding_register_variable(18, ModbusRegisterValue::DataType::UINT32_LM);
	auto register26 = modbusIpProcess.get_holding_register_variable(26, ModbusRegisterValue::DataType::UINT16);

	modbusIpProcess.start();
	int i = 0;
	while(1) {
		this_thread::sleep_for(10s);
		printf("Loop %d\n", i++);
		printf("coil 1 value: %s\n", coil1->get_value().to_string().c_str());
		printf("coil 2 value: %s\n", coil2->get_value().to_string().c_str());
		printf("coil 5 value: %s\n", coil5->get_value().to_string().c_str());
		printf("coil 7 value: %s\n", coil7->get_value().to_string().c_str());
		printf("coil 19 value: %s\n", coil19->get_value().to_string().c_str());
		printf("coil 30 value: %s\n", coil30->get_value().to_string().c_str());
		printf("coil 48 value: %s\n", coil48->get_value().to_string().c_str());
		printf("\n");
		printf("register 4 value: %s\n", register4->get_value().to_string().c_str());
		printf("register 8 value: %s\n", register8->get_value().to_string().c_str());
		printf("register 9 value: %s\n", register9->get_value().to_string().c_str());
		printf("register 12 value: %s\n", register12->get_value().to_string().c_str());
		printf("register 14 value: %s\n", register14->get_value().to_string().c_str());
		printf("register 18 value: %s\n", register18->get_value().to_string().c_str());
		printf("register 26 value: %s\n", register26->get_value().to_string().c_str());
		printf("\n");
		printf("\n");
	}
}

void test_modbus_rtu_process() {
	shared_ptr<SyncedSerialPort> serialPort = make_shared<SyncedSerialPort>();
	SyncedSerialPort::Config serialConfig;
	serialConfig.parityBit = false;
	serialConfig.stopBit = false;
	serialConfig.bitPerByte = 8;
	serialConfig.hardwareFlowControl = false;
	serialConfig.softwareFlowControl = false;
	serialConfig.baudrate = 19200;
	serialPort->open("/dev/ttyUSB0", serialConfig);

	ModbusRtuProcess modbusRtuProcess(serialPort, 1, 16, 64, chrono::milliseconds(500), true);
	
	auto coil0 = modbusRtuProcess.create_coil_status_variable(0);
	auto coil2 = modbusRtuProcess.create_coil_status_variable(2);
	auto coil5 = modbusRtuProcess.create_coil_status_variable(5);
	auto coil7 = modbusRtuProcess.create_coil_status_variable(7);
	auto coil19 = modbusRtuProcess.create_coil_status_variable(19);
	auto coil30 = modbusRtuProcess.create_coil_status_variable(30);
	auto coil48 = modbusRtuProcess.create_coil_status_variable(48);

	auto register1 = modbusRtuProcess.create_holding_register_variable(1, ModbusRegisterValue::DataType::INT16);
	auto register4 = modbusRtuProcess.create_holding_register_variable(4, ModbusRegisterValue::DataType::INT32_LM);
	auto register8 = modbusRtuProcess.create_holding_register_variable(8, ModbusRegisterValue::DataType::INT16);
	auto register9 = modbusRtuProcess.create_holding_register_variable(9, ModbusRegisterValue::DataType::INT32_LM);
	auto register12 = modbusRtuProcess.create_holding_register_variable(12, ModbusRegisterValue::DataType::INT32_LM);
	auto register14 = modbusRtuProcess.create_holding_register_variable(14, ModbusRegisterValue::DataType::INT64_LM);
	auto register18 = modbusRtuProcess.create_holding_register_variable(18, ModbusRegisterValue::DataType::UINT32_LM);
	auto register26 = modbusRtuProcess.create_holding_register_variable(26, ModbusRegisterValue::DataType::UINT16);

	modbusRtuProcess.start();
	int i = 0;
	while(1) {
		this_thread::sleep_for(10s);
		printf("Loop %d\n", i++);
		printf("coil 0 value: %s\n", coil0->read_value().to_string().c_str());
		printf("coil 2 value: %s\n", coil2->read_value().to_string().c_str());
		printf("coil 5 value: %s\n", coil5->read_value().to_string().c_str());
		printf("coil 7 value: %s\n", coil7->read_value().to_string().c_str());
		printf("coil 19 value: %s\n", coil19->read_value().to_string().c_str());
		printf("coil 30 value: %s\n", coil30->read_value().to_string().c_str());
		printf("coil 48 value: %s\n", coil48->read_value().to_string().c_str());
		printf("\n");
		printf("register 1 value: %s\n", register1->read_value().to_string().c_str());
		printf("register 4 value: %s\n", register4->read_value().to_string().c_str());
		printf("register 8 value: %s\n", register8->read_value().to_string().c_str());
		printf("register 9 value: %s\n", register9->read_value().to_string().c_str());
		printf("register 12 value: %s\n", register12->read_value().to_string().c_str());
		printf("register 14 value: %s\n", register14->read_value().to_string().c_str());
		printf("register 18 value: %s\n", register18->read_value().to_string().c_str());
		printf("register 26 value: %s\n", register26->read_value().to_string().c_str());
		printf("\n");
		printf("\n");
	}
}

/*
void test_frame()
{
	auto result = ModbusRTU::create_read_coil_status((uint8_t)0x11, (uint16_t)0x13, (uint16_t)0x25);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_read_input_status((uint8_t)0x11, (uint16_t)0xC4, (uint16_t)0x16);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_read_holding_register((uint8_t)0x11, (uint16_t)0x6B, (uint16_t)0x3);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_read_input_register((uint8_t)0x11, (uint16_t)0x08, (uint16_t)0x1);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_force_single_coil((uint8_t)0x11, (uint16_t)0xAC, true);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_preset_single_register((uint8_t)0x11, (uint16_t)0x01, (uint16_t)0x3);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_force_multiple_coils((uint8_t)0x11, (uint16_t)0x13, (uint16_t)0x0A, (uint8_t)0xCD, (uint8_t)0x01);
	PrintModbus::print_value_in_hex(result.first);
	result = ModbusRTU::create_preset_multiple_registers((uint8_t)0x11, (uint16_t)0x1, (uint16_t)0x02, (uint16_t)0xA, (uint16_t)0x102);
	PrintModbus::print_value_in_hex(result.first);

	ModbusRTU::ReplyData data = ModbusRTU::decode_reply(string("\x11\x01\x05\xCD\x6B\xB2\x0E\x1B\x45\xE6", 10));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x02\x03\xAC\xDB\x35\x20\x18", 8));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x03\x06\xAE\x41\x56\x52\x43\x40\x49\xAD", 11));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x04\x02\x00\x0A\xF8\xF4", 7));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x05\x00\xAC\xFF\x00\x4E\x8B", 8));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x06\x00\x01\x00\x03\x9A\x9B", 8));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x0F\x00\x13\x00\x0A\x26\x99", 8));
	PrintModbus::print_modbus_reply(data);
	data = ModbusRTU::decode_reply(string("\x11\x10\x00\x01\x00\x02\x12\x98", 8));
	PrintModbus::print_modbus_reply(data);
}
*/

#endif
