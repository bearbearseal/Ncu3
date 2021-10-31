OBJDIR = Obj/
OBJS = $(OBJDIR)main.o $(OBJDIR)TcpSocket.o $(OBJDIR)TcpListener.o
OBJS += $(OBJDIR)TcpTalker.o $(OBJDIR)TreeBrowser.o
OBJS += $(OBJDIR)Value.o $(OBJDIR)Variable.o $(OBJDIR)VariableTree.o
OBJS += $(OBJDIR)ModbusIP.o $(OBJDIR)ModbusIpProcess.o $(OBJDIR)ModbusRtu.o $(OBJDIR)ModbusRtuProcess.o $(OBJDIR)ModbusRegisterValue.o
OBJS += $(OBJDIR)OperationalLogic.o $(OBJDIR)OpCreator.o $(OBJDIR)StringManipulator.o $(OBJDIR)FileIOer.o $(OBJDIR)OpStorage.o
OBJS += $(OBJDIR)SerialPort.o $(OBJDIR)SyncedSerialPort.o $(OBJDIR)PrioritizedValue.o 
OBJS += $(OBJDIR)ScheduleRule.o $(OBJDIR)Schedule.o $(OBJDIR)TimeTable.o $(OBJDIR)Timer.o $(OBJDIR)ScheduleFunction.o
OBJS += $(OBJDIR)Sqlite3.o $(OBJDIR)Sqlite3JsonTalker.o $(OBJDIR)Sqlite3UdpListener.o
OBJS += $(OBJDIR)AlarmLogicGroup.o $(OBJDIR)AlarmProcessor.o $(OBJDIR)AlarmVerifyStorePost.o $(OBJDIR)AlarmStateChecker.o
OBJS += $(OBJDIR)AlarmTalker.o
OBJS += $(OBJDIR)UdpSocket.o $(OBJDIR)UdpListener.o
#OBJS += Equipment.o Property.o EquipmentManager.o
OBJS += $(OBJDIR)ConfigStorage.o $(OBJDIR)ChannelManager.o $(OBJDIR)SerialPortManager.o $(OBJDIR)ScheduleManager.o
CC = g++ -std=c++17
CFLAGS = -Wall -O2 -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

$(OBJDIR)main.o : main.cpp Deploy.hpp
	$(CC) $(CFLAGS) main.cpp -o $@

$(OBJDIR)TcpSocket.o: ../MyLib/TcpSocket/TcpSocket.h ../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpSocket.cpp -o $@

$(OBJDIR)TcpListener.o: ../MyLib/TcpSocket/TcpListener.h ../MyLib/TcpSocket/TcpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/TcpSocket/TcpListener.cpp -o $@

$(OBJDIR)TcpTalker.o: VariableTree/TcpTalker.h VariableTree/TcpTalker.cpp
	$(CC) $(CFLAGS) VariableTree/TcpTalker.cpp -o $@

$(OBJDIR)TreeBrowser.o: VariableTree/TreeBrowser.h VariableTree/TreeBrowser.cpp
	$(CC) $(CFLAGS) VariableTree/TreeBrowser.cpp -o $@

$(OBJDIR)Value.o: ../MyLib/Basic/Value.h ../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/Value.cpp -o $@

$(OBJDIR)PrioritizedValue.o: ../MyLib/Basic/PrioritizedValue.h ../MyLib/Basic/PrioritizedValue.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/PrioritizedValue.cpp -o $@

$(OBJDIR)Variable.o: ../MyLib/Basic/Variable.h ../MyLib/Basic/Variable.cpp
	$(CC) $(CFLAGS) ../MyLib/Basic/Variable.cpp -o $@

$(OBJDIR)VariableTree.o: VariableTree/VariableTree.h VariableTree/VariableTree.cpp ../MyLib/Basic/RamVariable.h
	$(CC) $(CFLAGS) VariableTree/VariableTree.cpp -o $@

$(OBJDIR)ModbusIP.o: Modbus/ModbusIP.h Modbus/ModbusIP.cpp
	$(CC) $(CFLAGS) Modbus/ModbusIP.cpp -o $@

$(OBJDIR)ModbusIpProcess.o: Modbus/ModbusIpProcess.h Modbus/ModbusIpProcess.cpp
	$(CC) $(CFLAGS) Modbus/ModbusIpProcess.cpp -o $@

$(OBJDIR)ModbusRtu.o: Modbus/ModbusRtu.h Modbus/ModbusRtu.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRtu.cpp -o $@

$(OBJDIR)ModbusRtuProcess.o: Modbus/ModbusRtuProcess.h Modbus/ModbusRtuProcess.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRtuProcess.cpp -o $@

$(OBJDIR)ModbusRegisterValue.o: Modbus/ModbusRegisterValue.h Modbus/ModbusRegisterValue.cpp
	$(CC) $(CFLAGS) Modbus/ModbusRegisterValue.cpp -o $@

$(OBJDIR)SerialPort.o: ../MyLib/SerialPort/SerialPort.h ../MyLib/SerialPort/SerialPort.cpp
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SerialPort.cpp -o $@

$(OBJDIR)SyncedSerialPort.o: ../MyLib/SerialPort/SyncedSerialPort.h ../MyLib/SerialPort/SyncedSerialPort.cpp
	$(CC) $(CFLAGS) ../MyLib/SerialPort/SyncedSerialPort.cpp -o $@

$(OBJDIR)AlarmLogicGroup.o: Alarm/AlarmLogicGroup.h Alarm/AlarmLogicGroup.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmLogicGroup.cpp -o $@

$(OBJDIR)AlarmProcessor.o: Alarm/AlarmProcessor.h Alarm/AlarmProcessor.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmProcessor.cpp -o $@

$(OBJDIR)AlarmVerifyStorePost.o: Alarm/AlarmVerifyStorePost.h Alarm/AlarmVerifyStorePost.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmVerifyStorePost.cpp -o $@

$(OBJDIR)AlarmStateChecker.o: Alarm/AlarmStateChecker.h Alarm/AlarmStateChecker.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmStateChecker.cpp -o $@

$(OBJDIR)AlarmTalker.o: Alarm/AlarmTalker.h Alarm/AlarmTalker.cpp Alarm/AlarmDefinition.h
	$(CC) $(CFLAGS) Alarm/AlarmTalker.cpp -o $@

$(OBJDIR)UdpSocket.o: ../MyLib/UdpSocket/UdpSocket.h ../MyLib/UdpSocket/UdpSocket.cpp
	$(CC) $(CFLAGS) ../MyLib/UdpSocket/UdpSocket.cpp -o $@

$(OBJDIR)UdpListener.o: ../MyLib/UdpSocket/UdpListener.h ../MyLib/UdpSocket/UdpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/UdpSocket/UdpListener.cpp -o $@

$(OBJDIR)Sqlite3.o: ../MyLib/Sqlite/Sqlite3.h ../MyLib/Sqlite/Sqlite3.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3.cpp -o $@

$(OBJDIR)Sqlite3JsonTalker.o: ../MyLib/Sqlite/Sqlite3JsonTalker.h ../MyLib/Sqlite/Sqlite3JsonTalker.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3JsonTalker.cpp -o $@

$(OBJDIR)Sqlite3UdpListener.o: ../MyLib/Sqlite/Sqlite3UdpListener.h ../MyLib/Sqlite/Sqlite3UdpListener.cpp
	$(CC) $(CFLAGS) ../MyLib/Sqlite/Sqlite3UdpListener.cpp -o $@

#Builder.o: Builder/Builder.h Builder/Builder.cpp
#	$(CC) $(CFLAGS) Builder/Builder.cpp

$(OBJDIR)OperationalLogic.o: InOutOperation/OperationalLogic.h InOutOperation/OperationalLogic.cpp
	$(CC) $(CFLAGS) InOutOperation/OperationalLogic.cpp -o $@

$(OBJDIR)OpCreator.o: InOutOperation/OpCreator.h InOutOperation/OpCreator.cpp
	$(CC) $(CFLAGS) InOutOperation/OpCreator.cpp -o $@

$(OBJDIR)OpStorage.o: InOutOperation/OpStorage.h InOutOperation/OpStorage.cpp
	$(CC) $(CFLAGS) InOutOperation/OpStorage.cpp -o $@

$(OBJDIR)StringManipulator.o: ../MyLib/StringManipulator/StringManipulator.h ../MyLib/StringManipulator/StringManipulator.cpp
	$(CC) $(CFLAGS) ../MyLib/StringManipulator/StringManipulator.cpp -o $@

$(OBJDIR)FileIOer.o: ../MyLib/File/FileIOer.h ../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../MyLib/File/FileIOer.cpp -o $@

#Equipment.o: Equipment/Equipment.h Equipment/Equipment.cpp
#	$(CC) $(CFLAGS) Equipment/Equipment.cpp

#Property.o: Equipment/Property.h Equipment/Property.cpp
#	$(CC) $(CFLAGS) Equipment/Property.cpp

$(OBJDIR)ConfigStorage.o: Storage/ConfigStorage.h Storage/ConfigStorage.cpp
	$(CC) $(CFLAGS) Storage/ConfigStorage.cpp -o $@

$(OBJDIR)ScheduleRule.o: Schedule/ScheduleRule.h Schedule/ScheduleRule.cpp
	$(CC) $(CFLAGS) Schedule/ScheduleRule.cpp -o $@

$(OBJDIR)Schedule.o: Schedule/Schedule.h Schedule/Schedule.cpp
	$(CC) $(CFLAGS) Schedule/Schedule.cpp -o $@

$(OBJDIR)TimeTable.o: Schedule/TimeTable.h Schedule/TimeTable.cpp
	$(CC) $(CFLAGS) Schedule/TimeTable.cpp -o $@

$(OBJDIR)Timer.o: Schedule/Timer.h Schedule/Timer.cpp
	$(CC) $(CFLAGS) Schedule/Timer.cpp -o $@

$(OBJDIR)ScheduleFunction.o: Schedule/ScheduleFunction.h Schedule/ScheduleFunction.cpp
	$(CC) $(CFLAGS) Schedule/ScheduleFunction.cpp -o $@

$(OBJDIR)ChannelManager.o: Integrator/ChannelManager.h Integrator/ChannelManager.cpp
	$(CC) $(CFLAGS) Integrator/ChannelManager.cpp -o $@

#$(OBJDIR)EquipmentManager.o: Integrator/EquipmentManager.h Integrator/EquipmentManager.cpp
#	$(CC) $(CFLAGS) Integrator/EquipmentManager.cpp

$(OBJDIR)SerialPortManager.o: Integrator/SerialPortManager.h Integrator/SerialPortManager.cpp
	$(CC) $(CFLAGS) Integrator/SerialPortManager.cpp -o $@

$(OBJDIR)ScheduleManager.o: Integrator/ScheduleManager.h Integrator/ScheduleManager.cpp
	$(CC) $(CFLAGS) Integrator/ScheduleManager.cpp -o $@

clean:
	rm -f $(binaries) $(OBJDIR)*.o