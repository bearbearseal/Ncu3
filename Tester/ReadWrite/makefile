OBJS = main.o TcpSocket.o
CC = g++ -std=c++17
CFLAGS = -Wall -O2 -c
LFLAGS = -Wall
LIBS = -lpthread
#INC = -I../Basic
binaries = ReadWrite.bin

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

TcpSocket.o : ../../../MyLib/TcpSocket/TcpSocket.h ../../../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../../../MyLib/TcpSocket/TcpSocket.cpp

clean:
	rm -f $(binaries) *.o
