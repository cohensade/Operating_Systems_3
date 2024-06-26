CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pg
LDFLAGS = -pg
EXEC_LIST = list
EXEC_STACK = stack
SOURCES_LIST = q2withlist.cpp
SOURCES_STACK = q2withstack.cpp
OBJECTS_LIST = $(SOURCES_LIST:.cpp=.o)
OBJECTS_STACK = $(SOURCES_STACK:.cpp=.o)

all: $(EXEC_LIST) $(EXEC_STACK)

$(EXEC_LIST): $(OBJECTS_LIST)
	$(CXX) $(LDFLAGS) $(OBJECTS_LIST) -o $(EXEC_LIST)

$(EXEC_STACK): $(OBJECTS_STACK)
	$(CXX) $(LDFLAGS) $(OBJECTS_STACK) -o $(EXEC_STACK)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC_LIST) $(EXEC_STACK) $(OBJECTS_LIST) $(OBJECTS_STACK) gmon.out
