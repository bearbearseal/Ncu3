OBJS = main.o TcpSocket.o TcpListener.o TcpTalker.o TreeBrowser.o Value.o Variable.o VariableTree.o
OBJS += ModbusIP.o ModbusIpProcess.o ModbusRtu.o ModbusRtuProcess.o ModbusRegisterValue.o
OBJS += SerialPort.o SyncedSerialPort.o PrioritizedValue.o
CC = g++ -std=c++17
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread
#INC = -I../Basic
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp Test.hpp
	$(CC) $(CFLAGS) main.cpp

TcpSocket.o : ../MyLib/TcpSocket/TcpSocket.h ../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpSocket.cpp

TcpListener.o: ../MyLib/TcpSocket/TcpListener.h ../MyLib/TcpSocket/TcpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpListener.cpp

TcpTalker.o: VariableTree/TcpTalker.h VariableTree/TcpTalker.cpp
	$(CC) $(CFLAGS) VariableTree/TcpTalker.cpp

TreeBrowser.o: VariableTree/TreeBrowser.h VariableTree/TreeBrowser.cpp
	$(CC) $(CFLAGS) VariableTree/TreeBrowser.cpp

Value.o: Basic/Value.h Basic/Value.cpp
	$(CC) $(CFLAGS) Basic/Value.cpp

PrioritizedValue.o: Basic/PrioritizedValue.h Basic/PrioritizedValue.cpp
	$(CC) $(CFLAGS) Basic/PrioritizedValue.cpp

Variable.o: Basic/Variable.h Basic/Variable.cpp
	$(CC) $(CFLAGS) Basic/Variable.cpp

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

clean:
	rm -f $(binaries) *.o