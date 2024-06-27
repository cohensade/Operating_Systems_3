CC = g++
CFLAGS = -std=c++11 -Wall

main: q3.o
	$(CC) $(CFLAGS) q3.o -o main

main.o: q3.cpp
	$(CC) $(CFLAGS) -c q3.cpp

clean:
	rm -rf *.o main
