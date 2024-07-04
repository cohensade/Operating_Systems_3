#ifndef Proactor_HPP
#define Proactor_HPP

#include <iostream>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "Graph.hpp" // Include the header file for your Graph class

class Proactor {
private:
    fd_set master_set; // Master file descriptor set
    int max_fd; // Maximum file descriptor number
    bool running; // Flag to indicate if Proactor is running
    std::unordered_map<int, std::function<void(int)>> handlers; // Map of file descriptors to handler functions
    std::mutex graphMutex; // Mutex for synchronizing graph operations

public:
    Proactor();

    // Method to start the Proactor loop
    void* startProactor();

    // Method to add a file descriptor and handler function to the Proactor
    int addFdToProactor(int fd, std::function<void(int)> func);

    // Method to remove a file descriptor from the Proactor
    int removeFdFromProactor(int fd);

    // Method to stop the Proactor loop
    int stopProactor();

    // Method to handle client commands (you can define this in Proactor.cpp)
    void handleClient(int clientSocket, Graph& graph);

    // Method to start a proactor thread
    pthread_t startProactor(int sockfd, void* (*proactorFunc)(void*));

    // Method to stop a proactor thread
    int stopProactor(pthread_t tid);
};

#endif // Proactor_HPP
