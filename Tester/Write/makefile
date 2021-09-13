OBJS = main.o TcpSocket.o ArgumentExtractor.o FileIOer.o
CC = g++ -std=c++17
CFLAGS = -Wall -O2 -c
LFLAGS = -Wall
LIBS = -lpthread
#INC = -I../Basic
binaries = Write.bin

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp
	$(CC) $(CFLAGS) main.cpp

TcpSocket.o : ../../../MyLib/TcpSocket/TcpSocket.h ../../../MyLib/TcpSocket/TcpSocket.cpp
	$(CC) $(CFLAGS) ../../../MyLib/TcpSocket/TcpSocket.cpp

ArgumentExtractor.o: ../../../MyLib/ArgumentExtractor/ArgumentExtractor.h ../../../MyLib/ArgumentExtractor/ArgumentExtractor.cpp
	$(CC) $(CFLAGS) ../../../MyLib/ArgumentExtractor/ArgumentExtractor.cpp

FileIOer.o: ../../../MyLib/File/FileIOer.h ../../../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../../../MyLib/File/FileIOer.cpp

clean:
	rm -f $(binaries) *.o
