CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# Default target to build the main executable
all: main

# Link object files to create the main executable
main: main.o reactor.o
	$(CXX) $(CXXFLAGS) -o main main.o reactor.o

# Compile main.cpp to main.o
main.o: main.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

# Compile reactor.cpp to reactor.o
reactor.o: reactor.cpp reactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor.cpp

# Clean target to remove generated files
clean:
	rm -f main *.o
