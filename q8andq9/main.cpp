#include "proactor.hpp"
#include "Graph.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex> // Include mutex for thread safety

Graph graph; // Global graph instance

std::mutex graphMutex; // Mutex to protect graph modifications

// Function to handle client communication asynchronously
void* proactorFunc(void* sockfdPtr) {
    int clientSocket = *((int*)sockfdPtr);
    std::cout << "[Proactor] Handling client on socket fd " << clientSocket << std::endl;

    Proactor Proactor; // Create an instance of Proactor to manage file descriptors

    // Pass both clientSocket and graph to handleClient
    Proactor.handleClient(clientSocket, graph); // Call handleClient on Proactor instance
    
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

    Proactor Proactor; // Create an instance of Proactor to manage file descriptors

    // Add the server socket to the Proactor to accept new connections
    Proactor.addFdToProactor(serverSocket, [&](int fd) {
        int clientSocket;
        // Accept a new connection from a client
        if ((clientSocket = accept(fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << "[Server] New connection, socket fd is " << clientSocket << std::endl;
        // Start a new proactor to handle client communication asynchronously
        pthread_t proactorThread;
        pthread_create(&proactorThread, nullptr, proactorFunc, &clientSocket);
        pthread_detach(proactorThread); // Detach the proactor thread
        
    });

    // Start the Proactor to handle events on registered file descriptors
    std::cout << "[Server] Starting Proactor to handle events..." << std::endl;
    Proactor.startProactor();

    // Close the server socket when done serving clients
    close(serverSocket);
    std::cout << "[Server] Server socket closed" << std::endl;

    return 0; // Return 0 to indicate successful execution
}
