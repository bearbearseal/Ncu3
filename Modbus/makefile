OBJS = main.o SyncedSerialPort.o SerialPort.o ModbusRtuProcess.o 
OBJS += ModbusRegisterValue.o TcpSocket.o Value.o
OBJS += ModbusIpProcess.o Variable.o
OBJS += ModbusRtu.o PrintModbus.o ModbusIP.o
OBJS += OperationVariable.o OperationalLogic.o
#OBJS += Variable.o TcpSocket.o FileIOer.o #VariablesGateway.o
#OBJS += ModbusRTUChannel.o
CC = g++ -std=c++17 -Wno-psabi
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread
binaries = theMain.bin

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp Test.hpp
	$(CC) $(CFLAGS) main.cpp

SerialPort.o : ../../MyLib/SerialPort/SerialPort.h ../../MyLib/SerialPort/SerialPort.cpp
	$(CC) $(CFLAGS) ../../MyLib/SerialPort/SerialPort.cpp

SyncedSerialPort.o : ../../MyLib/SerialPort/SyncedSerialPort.h ../../MyLib/SerialPort/SyncedSerialPort.cpp 
	$(CC) $(CFLAGS) ../../MyLib/SerialPort/SyncedSerialPort.cpp

ModbusRtuProcess.o : ModbusRtuProcess.cpp ModbusRtuProcess.h
	$(CC) $(CFLAGS) ModbusRtuProcess.cpp

ModbusRegisterValue.o: ModbusRegisterValue.h ModbusRegisterValue.cpp
	$(CC) $(CFLAGS) ModbusRegisterValue.cpp
	
ModbusRtu.o: ModbusRtu.h ModbusRtu.cpp
	$(CC) $(CFLAGS) ModbusRtu.cpp

Value.o: ../../MyLib/Basic/Value.h ../../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/Value.cpp

PrintModbus.o: PrintModbus.h PrintModbus.cpp
	$(CC) $(CFLAGS) PrintModbus.cpp

Variable.o: ../../MyLib/Basic/Variable.h ../../MyLib/Basic/Variable.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/Variable.cpp
	
VariablesGateway.o: VariablesGateway.h VariablesGateway.cpp
	$(CC) $(CFLAGS) VariablesGateway.cpp

OperationVariable.o: ../InOutOperation/OperationVariable.h ../InOutOperation/OperationVariable.cpp
	$(CC) $(CFLAGS) ../InOutOperation/OperationVariable.cpp

OperationalLogic.o: ../InOutOperation/OperationalLogic.h ../InOutOperation/OperationalLogic.cpp
	$(CC) $(CFLAGS) ../InOutOperation/OperationalLogic.cpp

FileIOer.o: ../../MyLib/File/FileIOer.h  ../../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../../MyLib/File/FileIOer.cpp

TcpSocket.o: ../../MyLib/TcpSocket/TcpSocket.h ../../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../../MyLib/TcpSocket/TcpSocket.cpp

ModbusIP.o: ModbusRegisterValue.h ModbusIP.h ModbusIP.cpp
	$(CC) $(CFLAGS) ModbusIP.cpp

ModbusIpProcess.o: ModbusIpProcess.h ModbusIpProcess.cpp
	$(CC) $(CFLAGS) ModbusIpProcess.cpp

clean:
	rm -f $(binaries) *.o
