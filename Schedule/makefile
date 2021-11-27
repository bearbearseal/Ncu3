OBJS = main.o ScheduleRule.o Schedule.o
OBJS += TimeTable.o Timer.o ScheduleManager.o ScheduleFunction.o
OBJS += ConfigStorage.o Sqlite3.o GlobalEnum.o
OBJS += Value.o

CC = g++ -std=c++2a
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
#INC = -I../Basic
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp Test.hpp
	$(CC) $(CFLAGS) main.cpp

ScheduleRule.o: ScheduleRule.h ScheduleRule.cpp
	$(CC) $(CFLAGS) ScheduleRule.cpp

Schedule.o: Schedule.h Schedule.cpp
	$(CC) $(CFLAGS) Schedule.cpp

TimeTable.o: TimeTable.h TimeTable.cpp
	$(CC) $(CFLAGS) TimeTable.cpp

Timer.o: Timer.h Timer.cpp
	$(CC) $(CFLAGS) Timer.cpp

ScheduleFunction.o: ScheduleFunction.h ScheduleFunction.cpp
	$(CC) $(CFLAGS) ScheduleFunction.cpp

ScheduleManager.o: ScheduleManager.h ScheduleManager.cpp
	$(CC) $(CFLAGS) ScheduleManager.cpp

Value.o: ../../MyLib/Basic/Value.h ../../MyLib/Basic/Value.cpp
	$(CC) $(CFLAGS) ../../MyLib/Basic/Value.cpp

ConfigStorage.o: ../Storage/ConfigStorage.h ../Storage/ConfigStorage.cpp
	$(CC) $(CFLAGS) ../Storage/ConfigStorage.cpp

Sqlite3.o: ../../MyLib/Sqlite/Sqlite3.h ../../MyLib/Sqlite/Sqlite3.cpp
	$(CC) $(CFLAGS) ../../MyLib/Sqlite/Sqlite3.cpp

GlobalEnum.o: ../Global/GlobalEnum.h ../Global/GlobalEnum.cpp
	$(CC) $(CFLAGS) ../Global/GlobalEnum.cpp

clean:
	rm -f $(binaries) *.o
