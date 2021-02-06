BUILDDIR = Built
OBJS = main.o TcpSocket.o TcpListener.o TcpTalker.o TreeBrowser.o Value.o Variable.o VariableTree.o
OBJS += ModbusIP.o ModbusIpProcess.o ModbusRtu.o ModbusRtuProcess.o ModbusRegisterValue.o
OBJS += OperationalLogic.o OpCreator.o StringManipulator.o FileIOer.o OpStorage.o
OBJS += SerialPort.o SyncedSerialPort.o PrioritizedValue.o AlarmLogicConstant.o AlarmDetector.o
OBJS += Sqlite3.o Sqlite3JsonTalker.o Sqlite3UdpListener.o
OBJS += AlarmHandler.o UdpSocket.o UdpListener.o Builder.o
OBJS += Equipment.o Property.o
OBJS += ConfigStorage.o ChannelManager.o EquipmentManager.o SerialPortManager.o
CC = g++ -std=c++17
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
#INC = -I../Basic
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp Test.hpp Deploy.hpp
	$(CC) $(CFLAGS) main.cpp

TcpSocket.o : ../MyLib/TcpSocket/TcpSocket.h ../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpSocket.cpp

TcpListener.o: ../MyLib/TcpSocket/TcpListener.h ../MyLib/TcpSocket/TcpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpListener.cpp

TcpTalker.o: VariableTree/TcpTalker.h VariableTree/TcpTalker.cpp
	$(CC) $(CFLAGS) VariableTree/TcpTalker.cpp

TreeBrowser.o: VariableTree/TreeBrowser.h VariableTree/TreeBrowser.cpp
	$(CC) $(CFLAGS) VariableTree/TreeBrowser.cpp

Value.o: ../MyLib/Basic/Value.h ../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/Value.cpp

PrioritizedValue.o: ../MyLib/Basic/PrioritizedValue.h ../MyLib/Basic/PrioritizedValue.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/PrioritizedValue.cpp

Variable.o: ../MyLib/Basic/Variable.h ../MyLib/Basic/Variable.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/Variable.cpp

VariableTree.o: VariableTree/VariableTree.h VariableTree/VariableTree.cpp
	$(CC) $(CFLAGS) VariableTree/VariableTree.cpp

ModbusIP.o: Modbus/ModbusIP.h Modbus/ModbusIP.cpp
	$(CC) $(CFLAGS) Modbus/ModbusIP.cpp

ModbusIpProcess.o: Modbus/ModbusIpProcess.h Modbus/ModbusIpProcess.cpp
	$(CC) $(CFLAGS) Modbus/ModbusIpProcess.cpp

ModbusRtu.o: Modbus/ModbusRtu.h Modbus/ModbusRtu.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRtu.cpp

ModbusRtuProcess.o: Modbus/ModbusRtuProcess.h Modbus/ModbusRtuProcess.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRtuProcess.cpp

ModbusRegisterValue.o: Modbus/ModbusRegisterValue.h Modbus/ModbusRegisterValue.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRegisterValue.cpp

SerialPort.o: ../MyLib/SerialPort/SerialPort.h ../MyLib/SerialPort/SerialPort.cpp
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SerialPort.cpp

SyncedSerialPort.o: ../MyLib/SerialPort/SyncedSerialPort.h ../MyLib/SerialPort/SyncedSerialPort.cpp
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SyncedSerialPort.cpp

AlarmLogicConstant.o: Alarm/AlarmLogicConstant.h Alarm/AlarmLogicConstant.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmLogicConstant.cpp

AlarmDetector.o: Alarm/AlarmDetector.h Alarm/AlarmDetector.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmDetector.cpp

AlarmHandler.o: Alarm/AlarmHandler.h Alarm/AlarmHandler.cpp Alarm/AlarmDefinition.h ../MyLib/Basic/Helper.hpp
	$(CC) $(CFLAGS) Alarm/AlarmHandler.cpp

UdpSocket.o: ../MyLib/UdpSocket/UdpSocket.h ../MyLib/UdpSocket/UdpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/UdpSocket/UdpSocket.cpp

UdpListener.o: ../MyLib/UdpSocket/UdpListener.h ../MyLib/UdpSocket/UdpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/UdpSocket/UdpListener.cpp

Sqlite3.o: ../MyLib/Sqlite/Sqlite3.h ../MyLib/Sqlite/Sqlite3.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3.cpp

Sqlite3JsonTalker.o: ../MyLib/Sqlite/Sqlite3JsonTalker.h ../MyLib/Sqlite/Sqlite3JsonTalker.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3JsonTalker.cpp

Sqlite3UdpListener.o: ../MyLib/Sqlite/Sqlite3UdpListener.h ../MyLib/Sqlite/Sqlite3UdpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3UdpListener.cpp

Builder.o: Builder/Builder.h Builder/Builder.cpp
	$(CC) $(CFLAGS) Builder/Builder.cpp

OperationalLogic.o: InOutOperation/OperationalLogic.h InOutOperation/OperationalLogic.cpp
	$(CC) $(CFLAGS) InOutOperation/OperationalLogic.cpp

OpCreator.o: InOutOperation/OpCreator.h InOutOperation/OpCreator.cpp
	$(CC) $(CFLAGS) InOutOperation/OpCreator.cpp

OpStorage.o: InOutOperation/OpStorage.h InOutOperation/OpStorage.cpp
	$(CC) $(CFLAGS) InOutOperation/OpStorage.cpp

StringManipulator.o: ../MyLib/StringManipulator/StringManipulator.h ../MyLib/StringManipulator/StringManipulator.cpp
	$(CC) $(CFLAGS) ../MyLib/StringManipulator/StringManipulator.cpp

FileIOer.o: ../MyLib/File/FileIOer.h ../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../MyLib/File/FileIOer.cpp

Equipment.o: Equipment/Equipment.h Equipment/Equipment.cpp
	$(CC) $(CFLAGS) Equipment/Equipment.cpp

Property.o: Equipment/Property.h Equipment/Property.cpp
	$(CC) $(CFLAGS) Equipment/Property.cpp

ConfigStorage.o: Storage/ConfigStorage.h Storage/ConfigStorage.cpp
	$(CC) $(CFLAGS) Storage/ConfigStorage.cpp

ChannelManager.o: Integrator/ChannelManager.h Integrator/ChannelManager.cpp
	$(CC) $(CFLAGS) Integrator/ChannelManager.cpp

EquipmentManager.o: Integrator/EquipmentManager.h Integrator/EquipmentManager.cpp
	$(CC) $(CFLAGS) Integrator/EquipmentManager.cpp

SerialPortManager.o: Integrator/SerialPortManager.h Integrator/SerialPortManager.cpp
	$(CC) $(CFLAGS) Integrator/SerialPortManager.cpp

clean:
	rm -f $(binaries) *.o