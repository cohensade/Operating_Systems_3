#ifndef REACTOR_HPP
#define REACTOR_HPP

#include <functional>
#include <unordered_map>
#include <sys/select.h>

// Define a type alias for the reactor function
typedef std::function<void(int)> reactorFunc;

class Reactor {
public:
    Reactor();
    // Function to start the reactor
    void* startReactor();
    // Function to add a file descriptor to the reactor
    int addFdToReactor(int fd, reactorFunc func);
    // Function to remove a file descriptor from the reactor
    int removeFdFromReactor(int fd);
    // Function to stop the reactor
    int stopReactor();

private:
    // Map to store handler functions for each file descriptor
    std::unordered_map<int, reactorFunc> handlers;
    fd_set master_set; // Master file descriptor set
    int max_fd; // Maximum file descriptor number
    bool running; // Flag to indicate if the reactor is running
};

#endif // REACTOR_HPP
