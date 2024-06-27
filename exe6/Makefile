CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: server

server: graph.o reactor.o main.o
	$(CXX) $(CXXFLAGS) -o server graph.o reactor.o main.o

graph.o: graph.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c graph.cpp

reactor.o: reactor.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor.cpp

main.o: main.cpp graph.hpp reactor.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o server
