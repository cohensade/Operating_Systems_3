#include "Graph.hpp"
#include "proactor.hpp"
#include <iostream>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <condition_variable>

using namespace std;
Graph graph; // Global graph instance

std::mutex graphMutex; // Mutex to protect graph modifications
std::mutex conditionMutex;
std::condition_variable cond;
bool mostGraphConnected = false;
bool notLongerMajority = false;
bool wasMajority = false;

// Function to handle client communication asynchronously
void* proactorFunc(void* sockfdPtr) {
    int clientSocket = *((int*)sockfdPtr);
    std::cout << "[Proactor] Handling client on socket fd " << clientSocket << std::endl;

    proactor proactor; // Create an instance of proactor to manage file descriptors

    // Pass both clientSocket and graph to handleClient
    proactor.handleClient(clientSocket, graph); // Call handleClient on proactor instance
    
    return nullptr;
}


// Main function to initialize and run the server
int main() {
    int serverSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create a socket for the server
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "[Server] Socket created successfully" << std::endl;

    // Set up the server address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9034);

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "[Server] Socket bound to address and port successfully" << std::endl;

    // Listen for incoming connections on the server socket
    if (listen(serverSocket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "[Server] Listening for incoming connections..." << std::endl;

    // Main loop to accept incoming connections and handle them in new threads
    while (true) {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        std::cout << "[Server] Accepted new connection on socket fd " << clientSocket << std::endl;

        pthread_t proactorThread;
        if (pthread_create(&proactorThread, nullptr, proactorFunc, (void*)&clientSocket) != 0) {
            perror("Failed to create proactorThread");
            close(clientSocket);
        }
    }

    // Close the server socket before exiting
    close(serverSocket);
    std::cout << "[Server] Server socket closed" << std::endl;
    return 0;
}
