CC = g++
CFLAGS = -std=c++11 -Wall

all:main

main: maina.o
	$(CC) $(CFLAGS) maina.o -o main

maina.o: maina.cpp
	$(CC) $(CFLAGS) -c maina.cpp

clean:
	rm -f *.o main
