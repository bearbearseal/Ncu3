OBJS = main.o TcpSocket.o FileThreadWriter.o LogThreadWriter.o
OBJS += PointWriter.o FileIOer.o Value.o 
CC = g++ -std=c++17
CFLAGS = -Wall -O2 -c
LFLAGS = -Wall
LIBS = -lpthread
#INC = -I../Basic
binaries = WritePoint

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

TcpSocket.o : ../../../MyLib/TcpSocket/TcpSocket.h ../../../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../../../MyLib/TcpSocket/TcpSocket.cpp

FileThreadWriter.o: ../../../MyLib/File/FileThreadWriter.h ../../../MyLib/File/FileThreadWriter.cpp
	$(CC) $(CFLAGS) ../../../MyLib/File/FileThreadWriter.cpp

LogThreadWriter.o: ../../../MyLib/File/LogThreadWriter.h ../../../MyLib/File/LogThreadWriter.cpp
	$(CC) $(CFLAGS) ../../../MyLib/File/LogThreadWriter.cpp

FileIOer.o: ../../../MyLib/File/FileIOer.h ../../../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../../../MyLib/File/FileIOer.cpp

PointWriter.o: PointWriter.h PointWriter.cpp
	$(CC) $(CFLAGS) PointWriter.cpp

Value.o: ../../../MyLib/Basic/Value.h ../../../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../../../MyLib/Basic/Value.cpp

clean:
	rm -f $(binaries) *.o
