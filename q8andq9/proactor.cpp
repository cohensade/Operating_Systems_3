// Include necessary headers
#include "proactor.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include "Graph.hpp" 
#include <sys/socket.h>

// Constructor definition
Proactor::Proactor() : max_fd(0), running(false) {
    FD_ZERO(&master_set); // Initialize the master file descriptor set
}

// Method to start the Proactor loop
void* Proactor::startProactor() {
    running = true;
    std::cout << "[Proactor] Proactor started" << std::endl;
    while (running) {
        fd_set read_fds = master_set;
        // Use select to wait for activity on any of the file descriptors
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);

        if (activity < 0) {
            perror("select");
            std::cerr << "[Proactor] Error in select()" << std::endl;
            return nullptr;
        }

        // Check each file descriptor for activity
        for (int fd = 0; fd <= max_fd; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                auto it = handlers.find(fd);
                if (it != handlers.end()) {
                    std::cout << "[Proactor] Handling activity on fd " << fd << std::endl;
                    it->second(fd); // Call the handler function
                }
            }
        }
    }
    std::cout << "[Proactor] Proactor stopped" << std::endl;
    return nullptr;
}

// Method to add a file descriptor and handler function to the Proactor
int Proactor::addFdToProactor(int fd, std::function<void(int)> func) {
    if (handlers.find(fd) != handlers.end()) {
        std::cerr << "[Proactor] Error: fd " << fd << " already exists in Proactor" << std::endl;
        return -1; // fd already exists
    }
    handlers[fd] = func; // Add handler function to map
    FD_SET(fd, &master_set); // Add fd to master set
    if (fd > max_fd) {
        max_fd = fd; // Update max_fd if necessary
    }
    std::cout << "[Proactor] Added fd " << fd << " to Proactor" << std::endl;
    return 0;
}

// Method to remove a file descriptor from the Proactor
int Proactor::removeFdFromProactor(int fd) {
    if (handlers.find(fd) == handlers.end()) {
        std::cerr << "[Proactor] Error: fd " << fd << " not found in Proactor" << std::endl;
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
    std::cout << "[Proactor] Removed fd " << fd << " from Proactor" << std::endl;
    return 0;
}

// Method to stop the Proactor loop
int Proactor::stopProactor() {
    running = false; // Set running to false to exit the loop
    std::cout << "[Proactor] Stopping Proactor" << std::endl;
    return 0;
}

// Method to start a proactor thread
pthread_t Proactor::startProactor(int sockfd, void* (*proactorFunc)(void*)) {
    pthread_t proactorThread;
    pthread_create(&proactorThread, nullptr, proactorFunc, (void*)&sockfd);
    std::cout << "[Proactor] Started proactor thread for sockfd " << sockfd << std::endl;
    return proactorThread;
}

// Method to stop a proactor thread
int Proactor::stopProactor(pthread_t tid) {
    pthread_cancel(tid);
    pthread_join(tid, nullptr);
    std::cout << "[Proactor] Stopped proactor thread" << std::endl;
    return 0;
}

// Method to handle client commands
void Proactor::handleClient(int clientSocket, Graph& graph) {
    char buffer[1024];

    while (true) {
        // Read data from client
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "[Server] Received: " << buffer << std::endl;

            // Parse client commands and execute corresponding graph operations
            std::stringstream ss(buffer);
            std::string command;
            ss >> command;

            if (command == "Newgraph") {
                int n, m;
                ss >> n >> m;

                // Lock graph for Mutex
                std::lock_guard<std::mutex> lock(graphMutex);

                graph = Graph(n); // Reinitialize graph

                for (int i = 0; i < m; ++i) {
                    memset(buffer, 0, sizeof(buffer));
                    bytesRead = read(clientSocket, buffer, 1024);
                    if (bytesRead == 0) {
                        std::cout << "[Server] Client disconnected while sending graph edges." << std::endl;
                        close(clientSocket);
                        return;
                    }
                    std::stringstream edgeStream(buffer);
                    int u, v;
                    edgeStream >> u >> v;
                    graph.addEdge(u, v);
                }
            } else if (command == "Kosaraju") {
                // Lock graph for reading
                std::lock_guard<std::mutex> lock(graphMutex);

                std::vector<std::vector<int>> sccs = graph.kosaraju();

                std::stringstream response;
                response << "Kosaraju command executed.\nThe SCC's are:\n";
                for (const auto& scc : sccs) {
                    for (int node : scc) {
                        response << node << " ";
                    }
                    response << std::endl; // Print each SCC on a new line
                }
                send(clientSocket, response.str().c_str(), response.str().length(), 0);
            } else if (command == "Newedge") {
                int i, j;
                ss >> i >> j;
                std::cout << "[Server] Newedge command: adding edge " << i << " -> " << j << std::endl;

                // Lock graph for modification
                std::lock_guard<std::mutex> lock(graphMutex);

                graph.addEdge(i, j);
                send(clientSocket, "Newedge command executed successfully.\n", strlen("Newedge command executed successfully.\n"), 0);
            } else if (command == "Removeedge") {
                int i, j;
                ss >> i >> j;
                std::cout << "[Server] Removeedge command: Removing edge " << i << " -> " << j << std::endl;

                // Lock graph for modification
                std::lock_guard<std::mutex> lock(graphMutex);

                graph.removeEdge(i, j);
                send(clientSocket, "Removeedge command executed successfully.\n", strlen("Removeedge command executed successfully.\n"), 0);
            } else if (command == "Quit") {
                std::cout << "[Server] Client requested to quit. Closing connection." << std::endl;
                close(clientSocket);
                return;
            } else {
                // Handle invalid command or end of input
                send(clientSocket, "Invalid command.\n", strlen("Invalid command.\n"), 0);
            }

            // Debug print current state of the graph
            {
                std::lock_guard<std::mutex> lock(graphMutex);
                graph.printGraph(); // Make sure you have a printGraph() method to visualize the graph state
            }
            std::cout << std::endl; // Add extra line for clarity
        } else if (bytesRead == 0) {
            std::cerr << "[Server] Client disconnected. Closing connection." << std::endl;
            close(clientSocket);
            return;
        } else {
            std::cerr << "[Server] Error: No data read from client, closing connection" << std::endl;
            close(clientSocket);
            return;
        }
    }
}

