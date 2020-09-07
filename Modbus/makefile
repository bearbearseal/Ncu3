OBJS = main.o SyncedSerialPort.o SerialPort.o ModbusRtuProcess.o 
OBJS += ModbusRegisterValue.o TcpSocket.o Value.o MapKey.o
OBJS += ModbusIpProcess.o Variable.o
OBJS += ModbusRTU.o PrintModbus.o ModbusIP.o
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

SerialPort.o : ../MyLib/SerialPort/SerialPort.h ../MyLib/SerialPort/SerialPort.cpp
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SerialPort.cpp

SyncedSerialPort.o : ../MyLib/SerialPort/SyncedSerialPort.h ../MyLib/SerialPort/SyncedSerialPort.cpp ../MyLib/SerialPort/SerialPort.h
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SyncedSerialPort.cpp

ModbusRtuProcess.o : ModbusRtuProcess.cpp ModbusRtuProcess.h ../MyLib/SerialPort/SyncedSerialPort.h ModbusRTU.h ../Basic/Variable.h ModbusSubscriber.h ModbusRegisterValue.h
	$(CC) $(CFLAGS) ModbusRtuProcess.cpp

ModbusRegisterValue.o: ModbusRegisterValue.h ModbusRegisterValue.cpp ../Basic/Value.h ModbusDefinition.h ../Basic/Sys.h
	$(CC) $(CFLAGS) ModbusRegisterValue.cpp
	
ModbusRTU.o: ModbusRTU.h ModbusRTU.cpp ModbusDefinition.h
	$(CC) $(CFLAGS) ModbusRTU.cpp

Value.o: ../Basic/Value.h ../Basic/Value.cpp ../Basic/MapKey.h
	$(CC) $(CFLAGS) ../Basic/Value.cpp

MapKey.o: ../Basic/MapKey.h ../Basic/MapKey.cpp
	$(CC) $(CFLAGS) ../Basic/MapKey.cpp
	
PrintModbus.o: PrintModbus.h PrintModbus.cpp
	$(CC) $(CFLAGS) PrintModbus.cpp

Variable.o: ../Basic/Variable.h ../Basic/Variable.cpp ../Basic/Value.h ../Basic/VariableSubscriber.h
	$(CC) $(CFLAGS) ../Basic/Variable.cpp
	
VariablesGateway.o: VariablesGateway.h VariablesGateway.cpp ../Basic/MapKey.h ../Basic/Variable.h ../Basic/VariableSubscriber.h ../Interface/VariablesKeeper.h
	$(CC) $(CFLAGS) VariablesGateway.cpp

FileIOer.o: ../MyLib/File/FileIOer.h  ../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../MyLib/File/FileIOer.cpp

TcpSocket.o: ../MyLib/TcpSocket/TcpSocket.h ../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpSocket.cpp

ModbusIP.o: ModbusRegisterValue.h ModbusIP.h ModbusIP.cpp
	$(CC) $(CFLAGS) ModbusIP.cpp

ModbusIpProcess.o: ModbusIpProcess.h ModbusIpProcess.cpp ModbusIP.h ../MyLib/TcpSocket/TcpSocket.h ../Basic/Variable.h
	$(CC) $(CFLAGS) ModbusIpProcess.cpp

clean:
	rm -f $(binaries) *.o
