OBJS = main.o TimeEventGun.o ScheduleRule.o

CC = g++ -std=c++17
CFLAGS = -Wall -c
LFLAGS = -Wall
LIBS = -lpthread -lsqlite3
#INC = -I../Basic
binaries = theMain.out

main : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o $(binaries)

main.o : main.cpp Test.hpp
	$(CC) $(CFLAGS) main.cpp

TimeEventGun.o : TimeEventGun.h TimeEventGun.cpp
	$(CC) $(CFLAGS) TimeEventGun.cpp

ScheduleRule.o: ScheduleRule.h ScheduleRule.cpp
	$(CC) $(CFLAGS) ScheduleRule.cpp

clean:
	rm -f $(binaries) *.o
