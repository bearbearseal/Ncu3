OBJS = FileAndSqliteEditor.o LoginJsonTalker.o LoginUdpListener.o
OBJS += Sqlite3.o Sqlite3JsonTalker.o Sqlite3UdpListener.o
OBJS += UdpSocket.o UdpListener.o
OBJS += FileIOer.o FileBrowser.o UdpFileBrowser.o
OBJS += StringManipulator.o
CC = g++ -std=c++17
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
binaries = FileAndSqliteEditor.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

FileAndSqliteEditor.o : FileAndSqliteEditor.cpp
	$(CC) $(CFLAGS) FileAndSqliteEditor.cpp

LoginJsonTalker.o : LoginJsonTalker.h LoginJsonTalker.cpp
	$(CC) $(CFLAGS) LoginJsonTalker.cpp

LoginUdpListener.o : LoginUdpListener.h LoginUdpListener.cpp
	$(CC) $(CFLAGS) LoginUdpListener.cpp

UdpSocket.o: ../../MyLib/UdpSocket/UdpSocket.h ../../MyLib/UdpSocket/UdpSocket.cpp
	$(CC) $(CFLAGS) ../../MyLib/UdpSocket/UdpSocket.cpp

UdpListener.o: ../../MyLib/UdpSocket/UdpListener.h ../../MyLib/UdpSocket/UdpListener.cpp
	$(CC) $(CFLAGS) ../../MyLib/UdpSocket/UdpListener.cpp

Sqlite3.o: ../../MyLib/Sqlite/Sqlite3.h ../../MyLib/Sqlite/Sqlite3.cpp
	$(CC) $(CFLAGS) ../../MyLib/Sqlite/Sqlite3.cpp

Sqlite3JsonTalker.o: ../../MyLib/Sqlite/Sqlite3JsonTalker.h ../../MyLib/Sqlite/Sqlite3JsonTalker.cpp
	$(CC) $(CFLAGS) ../../MyLib/Sqlite/Sqlite3JsonTalker.cpp

Sqlite3UdpListener.o: ../../MyLib/Sqlite/Sqlite3UdpListener.h ../../MyLib/Sqlite/Sqlite3UdpListener.cpp
	$(CC) $(CFLAGS) ../../MyLib/Sqlite/Sqlite3UdpListener.cpp

FileIOer.o: ../../MyLib/File/FileIOer.h ../../MyLib/File/FileIOer.cpp
	$(CC) $(CFLAGS) ../../MyLib/File/FileIOer.cpp

FileBrowser.o: ../../MyLib/File/FileBrowser.h ../../MyLib/File/FileBrowser.cpp
	$(CC) $(CFLAGS) ../../MyLib/File/FileBrowser.cpp

UdpFileBrowser.o: ../../MyLib/File/UdpFileBrowser.h ../../MyLib/File/UdpFileBrowser.cpp
	$(CC) $(CFLAGS) ../../MyLib/File/UdpFileBrowser.cpp

StringManipulator.o: ../../MyLib/StringManipulator/StringManipulator.h ../../MyLib/StringManipulator/StringManipulator.cpp
	$(CC) $(CFLAGS) ../../MyLib/StringManipulator/StringManipulator.cpp

clean:
	rm -f $(binaries) *.o