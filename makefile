OBJS = main.o TcpSocket.o TcpListener.o TcpTalker.o TreeBrowser.o Value.o Variable.o VariableTree.o
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

Variable.o: Basic/Variable.h Basic/Variable.cpp
	$(CC) $(CFLAGS) Basic/Variable.cpp

VariableTree.o: VariableTree/VariableTree.h VariableTree/VariableTree.cpp
	$(CC) $(CFLAGS) VariableTree/VariableTree.cpp

clean:
	rm -f $(binaries) *.o