#include "reactor.hpp"
#include "graph.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void handleClientCommands(int clientSocket, Graph& graph) {
    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    if (valread <= 0) {
        if (valread == 0) {
            std::cout << "Client disconnected." << std::endl;
        } else {
            perror("recv");
        }
        close(clientSocket);
        return;
    }

    std::stringstream ss(buffer);
    std::string command;
    ss >> command;

    if (command == "Newgraph") {
        int n, m;
        ss >> n >> m;
        graph = Graph(n); // Reinitialize graph

        for (int i = 0; i < m; ++i) {
            memset(buffer, 0, sizeof(buffer));
            valread = read(clientSocket, buffer, 1024);
            if (valread == 0) {
                std::cout << "Client disconnected while sending graph edges." << std::endl;
                close(clientSocket);
                return;
            }
            std::stringstream edgeStream(buffer);
            int u, v;
            edgeStream >> u >> v;
            graph.addEdge(u, v);
        }
    } else if (command == "Kosaraju") {
        std::vector<std::vector<int>> sccs = graph.kosaraju();

        std::stringstream response;
        response << "Kosaraju command executed.\nThe SCC's are:\n";
        for (const auto& scc : sccs) {
            for (int node : scc) {
                response << node << " ";
            }
            response << std::endl;
        }
        send(clientSocket, response.str().c_str(), response.str().length(), 0);
    } else if (command == "Newedge") {
        int i, j;
        ss >> i >> j;
        std::cout << "Newedge command: adding edge " << i << " -> " << j << std::endl;
        graph.addEdge(i, j);
        send(clientSocket, "Newedge command executed successfully.\n", strlen("Newedge command executed successfully.\n"), 0);
    } else if (command == "Removeedge") {
        int i, j;
        ss >> i >> j;
        std::cout << "Removeedge command: Removing edge " << i << " -> " << j << std::endl;
        graph.removeEdge(i, j);
        send(clientSocket, "Removeedge command executed successfully.\n", strlen("Removeedge command executed successfully.\n"), 0);
    } else {
        // Handle invalid command or end of input
        send(clientSocket, "Invalid command.\n", strlen("Invalid command.\n"), 0);
    }

    // Debug print current state of the graph
    graph.printGraph(); // Make sure you have a printGraph() method to visualize the graph state
    std::cout << std::endl; // Add extra line for clarity
}

void acceptConnection(int serverSocket, Reactor& reactor, Graph& graph) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int clientSocket;

    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    std::cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(address.sin_addr) << ", port : " << ntohs(address.sin_port) << std::endl;

    reactor.addFdToReactor(clientSocket, [&graph](int clientSocket) {
        handleClientCommands(clientSocket, graph);
    });
}

int main() {
    int serverSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    Graph graph(0); // Initialize graph with 0 nodes

    // Creating socket file descriptor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 9034
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9034);

    // Forcefully attaching socket to the port 9034
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    Reactor reactor;
    reactor.addFdToReactor(serverSocket, [&reactor, &graph](int serverSocket) {
        acceptConnection(serverSocket, reactor, graph);
    });

    std::cout << "Server opened and now listening for clients to connect" << std::endl;

    reactor.startReactor();

    return 0;
}
