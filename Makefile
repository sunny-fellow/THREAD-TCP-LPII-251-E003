CC=gcc
CFLAGS=-Wall -pthread -g

all: log_test

tslog.o: tslog.c tslog.h
	$(CC) $(CFLAGS) -c tslog.c

log_test: log_test.c tslog.o
	$(CC) $(CFLAGS) -o log_test log_test.c tslog.o

clean:
	rm -f *.o log_test test.log
