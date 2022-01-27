OBJS = main.o PointLogger.o PointLogStorage.o
OBJS += VariableTree.o EventTimer.o Variable.o Value.o Sqlite3.o 
OBJS += TcpTalker.o TreeBrowser.o TcpSocket.o TcpListener.o
OBJS += PrioritizedValue.o

CC = g++ -std=c++2a
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
#INC = -I../Basic
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

PointLogStorage.o: PointLogStorage.h PointLogStorage.cpp
	$(CC) $(CFLAGS) PointLogStorage.cpp

PointLogger.o: PointLogger.h PointLogger.cpp
	$(CC) $(CFLAGS) PointLogger.cpp

VariableTree.o: ../VariableTree/VariableTree.h ../VariableTree/VariableTree.cpp
	$(CC) $(CFLAGS) ../VariableTree/VariableTree.cpp

EventTimer.o: ../../MyLib/Timer/EventTimer.h ../../MyLib/Timer/EventTimer.cpp
	$(CC) $(CFLAGS) ../../MyLib/Timer/EventTimer.cpp

Variable.o: ../../MyLib/Basic/Variable.h ../../MyLib/Basic/Variable.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/Variable.cpp

PrioritizedValue.o: ../../MyLib/Basic/PrioritizedValue.h ../../MyLib/Basic/PrioritizedValue.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/PrioritizedValue.cpp

Value.o: ../../MyLib/Basic/Value.h ../../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/Value.cpp

#RamVariable.o: ../../MyLib/Basic/RamVariable.h ../../MyLib/Basic/RamVariable.cpp
#	$(CC) $(CFLAGS) ../../MyLib/Basic/RamVariable.cpp

Sqlite3.o: ../../MyLib/Sqlite/Sqlite3.h ../../MyLib/Sqlite/Sqlite3.cpp
	$(CC) $(CFLAGS) ../../MyLib/Sqlite/Sqlite3.cpp

TcpSocket.o: ../../MyLib/TcpSocket/TcpSocket.h ../../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../../MyLib/TcpSocket/TcpSocket.cpp

TcpListener.o: ../../MyLib/TcpSocket/TcpListener.h ../../MyLib/TcpSocket/TcpListener.cpp
	$(CC) $(CFLAGS) ../../MyLib/TcpSocket/TcpListener.cpp

TcpTalker.o: ../VariableTree/TcpTalker.h ../VariableTree/TcpTalker.cpp
	$(CC) $(CFLAGS) ../VariableTree/TcpTalker.cpp

TreeBrowser.o: ../VariableTree/TreeBrowser.h ../VariableTree/TreeBrowser.cpp
	$(CC) $(CFLAGS) ../VariableTree/TreeBrowser.cpp

clean:
	rm -f $(binaries) *.o
