CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread   # Added -pthread for pthread library

# Default target to build the main executable
all: main

# Link object files to create the main executable
main: main.o proactor.o Graph.o
	$(CXX) $(CXXFLAGS) -o main main.o proactor.o Graph.o

# Compile main.cpp to main.o
main.o: main.cpp proactor.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

# Compile proactor.cpp to proactor.o
proactor.o: proactor.cpp proactor.hpp
	$(CXX) $(CXXFLAGS) -c proactor.cpp

# Compile Graph.cpp to Graph.o
Graph.o: Graph.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) -c Graph.cpp

# Clean target to remove generated files
clean:
	rm -f main *.o

#how to kill port
#sudo ss -tuln | grep 9034