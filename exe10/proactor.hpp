#ifndef PROACTOR_HPP
#define PROACTOR_HPP

#include "Graph.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <pthread.h>
#include <sys/select.h>
#include <mutex>
#include <condition_variable>

// Declare extern for variables defined in main.cpp
extern Graph graph;
extern std::mutex graphMutex;
extern std::mutex conditionMutex;
extern std::condition_variable cond;
extern bool mostGraphConnected;
extern bool notLongerMajority;
extern bool wasMajority;

// proactor class definition
class proactor {
public:
    proactor(); // Constructor
    void* startproactor(); // Method to start the proactor loop
    int addFdToproactor(int fd, std::function<void(int)> func); // Method to add a file descriptor and handler function to the proactor
    int removeFdFromproactor(int fd); // Method to remove a file descriptor from the proactor
    int stopproactor(); // Method to stop the proactor loop
    pthread_t startProactor(int sockfd, void* (*proactorFunc)(void*)); // Method to start a proactor thread
    int stopProactor(pthread_t tid); // Method to stop a proactor thread
    void handleClient(int clientSocket, Graph& graph); // Method to handle client commands
    void printThread();
    
private:
    fd_set master_set; // Set of all file descriptors
    int max_fd; // Maximum file descriptor value
    bool running; // Flag to control the proactor loop
    std::unordered_map<int, std::function<void(int)>> handlers; // Map of file descriptors to handler functions
    std::mutex graphMutex; // Mutex for thread-safe graph operations
};

#endif // PROACTOR_HPP
