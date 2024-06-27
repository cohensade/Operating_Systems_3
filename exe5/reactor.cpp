#include "reactor.hpp"
#include <iostream>
#include <unistd.h>

// Constructor to initialize reactor
Reactor::Reactor() : max_fd(0), running(false) {
    FD_ZERO(&master_set); // Initialize the master file descriptor set
}

// Function to start the reactor
void* Reactor::startReactor() {
    running = true;
    while (running) {
        fd_set read_fds = master_set;
        // Use select to wait for activity on any of the file descriptors
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);

        if (activity < 0) {
            perror("select");
            return nullptr;
        }

        // Check each file descriptor for activity
        for (int fd = 0; fd <= max_fd; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                auto it = handlers.find(fd);
                if (it != handlers.end()) {
                    it->second(fd); // Call the handler function
                }
            }
        }
    }
    return nullptr;
}

// Function to add a file descriptor to the reactor
int Reactor::addFdToReactor(int fd, reactorFunc func) {
    if (handlers.find(fd) != handlers.end()) {
        return -1; // fd already exists
    }
    handlers[fd] = func; // Add handler function to map
    FD_SET(fd, &master_set); // Add fd to master set
    if (fd > max_fd) {
        max_fd = fd; // Update max_fd if necessary
    }
    return 0;
}

// Function to remove a file descriptor from the reactor
int Reactor::removeFdFromReactor(int fd) {
    if (handlers.find(fd) == handlers.end()) {
        return -1; // fd does not exist
    }
    handlers.erase(fd); // Remove handler function from map
    FD_CLR(fd, &master_set); // Remove fd from master set
    if (fd == max_fd) {
        // Update max_fd to the next highest fd
        while (max_fd > 0 && !FD_ISSET(max_fd, &master_set)) {
            --max_fd;
        }
    }
    return 0;
}

// Function to stop the reactor
int Reactor::stopReactor() {
    running = false; // Set running to false to exit the loop
    return 0;
}
