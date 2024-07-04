#include "reactor.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Function to handle client communication
void handleClient(int clientSocket) {
    char buffer[1024];
    // Read data from client
    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Received: " << buffer << std::endl;
        // write back to client
        write(clientSocket, buffer, bytesRead);
    } else {
        // Close the connection if no data is read
        close(clientSocket);
    }
}

int main() {
    int serverSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set address and port for the server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9034);

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    Reactor reactor;

    // Add server socket to reactor to accept new connections
    reactor.addFdToReactor(serverSocket, [&](int fd) {
        int clientSocket;
        // Accept a new connection          
        if ((clientSocket = accept(fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << "New connection, socket fd is " << clientSocket << std::endl;
        // Add new client socket to reactor to handle client commands
        reactor.addFdToReactor(clientSocket, handleClient);
    });

    // Start the reactor
    reactor.startReactor();

    // Close the server socket when done
    close(serverSocket);
    return 0;
}
