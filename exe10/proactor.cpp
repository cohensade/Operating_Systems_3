#include "proactor.hpp"
#include "Graph.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <condition_variable>

using namespace std;

// Constructor definition
proactor::proactor() : max_fd(0), running(false)
{
    FD_ZERO(&master_set); // Initialize the master file descriptor set
}

// Method to start the proactor loop
void *proactor::startproactor()
{
    running = true;
    std::cout << "[proactor] proactor started" << std::endl;
    while (running)
    {
        fd_set read_fds = master_set;
        // Use select to wait for activity on any of the file descriptors
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);

        if (activity < 0)
        {
            perror("select");
            std::cerr << "[proactor] Error in select()" << std::endl;
            return nullptr;
        }

        // Check each file descriptor for activity
        for (int fd = 0; fd <= max_fd; ++fd)
        {
            if (FD_ISSET(fd, &read_fds))
            {
                auto it = handlers.find(fd);
                if (it != handlers.end())
                {
                    std::cout << "[proactor] Handling activity on fd " << fd << std::endl;
                    it->second(fd); // Call the handler function
                }
            }
        }
    }
    std::cout << "[proactor] proactor stopped" << std::endl;
    return nullptr;
}

// Method to add a file descriptor and handler function to the proactor
int proactor::addFdToproactor(int fd, std::function<void(int)> func)
{
    if (handlers.find(fd) != handlers.end())
    {
        std::cerr << "[proactor] Error: fd " << fd << " already exists in proactor" << std::endl;
        return -1; // fd already exists
    }
    handlers[fd] = func;     // Add handler function to map
    FD_SET(fd, &master_set); // Add fd to master set
    if (fd > max_fd)
    {
        max_fd = fd; // Update max_fd if necessary
    }
    std::cout << "[proactor] Added fd " << fd << " to proactor" << std::endl;
    return 0;
}

// Method to remove a file descriptor from the proactor
int proactor::removeFdFromproactor(int fd)
{
    if (handlers.find(fd) == handlers.end())
    {
        std::cerr << "[proactor] Error: fd " << fd << " not found in proactor" << std::endl;
        return -1; // fd does not exist
    }
    handlers.erase(fd);      // Remove handler function from map
    FD_CLR(fd, &master_set); // Remove fd from master set
    if (fd == max_fd)
    {
        // Update max_fd to the next highest fd
        while (max_fd > 0 && !FD_ISSET(max_fd, &master_set))
        {
            --max_fd;
        }
    }
    std::cout << "[proactor] Removed fd " << fd << " from proactor" << std::endl;
    return 0;
}

// Method to stop the proactor loop
int proactor::stopproactor()
{
    running = false; // Set running to false to exit the loop
    std::cout << "[proactor] Stopping proactor" << std::endl;
    return 0;
}

// Method to start a proactor thread
pthread_t proactor::startProactor(int sockfd, void *(*proactorFunc)(void *))
{
    pthread_t proactorThread;
    pthread_create(&proactorThread, nullptr, proactorFunc, (void *)&sockfd);
    std::cout << "[Proactor] Started proactor thread for sockfd " << sockfd << std::endl;
    return proactorThread;
}

// Method to stop a proactor thread
int proactor::stopProactor(pthread_t tid)
{
    pthread_cancel(tid);
    pthread_join(tid, nullptr);
    std::cout << "[Proactor] Stopped proactor thread" << std::endl;
    return 0;
}

void proactor::printThread()
{
    std::unique_lock<std::mutex> lock(conditionMutex);
    while (true)
    {
        cond.wait(lock, []()
                  { return mostGraphConnected || notLongerMajority; });
        if (mostGraphConnected)
        {
            std::cout << "\nAt least 50% of the graph belongs to the same SCC\n"<<endl;
            
            mostGraphConnected = false;
        }
       
    }
}

// Method to handle client commands
void proactor::handleClient(int clientSocket, Graph &graph)
{
    char buffer[1024];

    while (true)
    {
        // Read data from client
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            std::cout << "[Server] Received: " << buffer << std::endl;

            // Parse client commands and execute corresponding graph operations
            std::stringstream ss(buffer);
            std::string command;
            ss >> command;

            if (command == "Newgraph")
            {
                int n, m;
                ss >> n >> m;

                // Lock graph for Mutex
                std::lock_guard<std::mutex> lock(graphMutex);

                graph = Graph(n); // Reinitialize graph
                                  // graph.clear(); // Clear existing graph data
                                  //  graph.resize(n); // Resize the graph

                // Update global condition
                std::lock_guard<std::mutex> lock2(conditionMutex);
                mostGraphConnected = true;
                notLongerMajority = false;
               

                // Read edges and update the graph
                for (int i = 0; i < m; ++i)
                {
                    memset(buffer, 0, sizeof(buffer));
                    bytesRead = read(clientSocket, buffer, 1024);
                    if (bytesRead == 0)
                    {
                        std::cout << "[Server] Client disconnected while sending graph edges." << std::endl;
                        close(clientSocket);
                        return;
                    }

                    std::stringstream edgeStream(buffer);
                    int u, v;
                    edgeStream >> u >> v;
                    graph.addEdge(u, v);
                }
                // Reset buffer and ensure subsequent commands are handled correctly
                memset(buffer, 0, sizeof(buffer));
            }

            if (command == "Kosaraju")
            {
                // Lock graph for reading
                std::lock_guard<std::mutex> lock(graphMutex);

                std::vector<std::vector<int>> sccs = graph.kosaraju();

                // Calculate the size of the largest SCC
                int maxSize = 0;
                for (const auto &scc : sccs)
                {
                    if (scc.size() > maxSize)
                    {
                        maxSize = scc.size();
                    }
                }

                // Determine if it's more than 50% of the graph
                mostGraphConnected = (maxSize >= graph.getNumVertices() / 2);
                notLongerMajority = !mostGraphConnected;
                 // Notify the print thread
                {
                    std::lock_guard<std::mutex> lock(conditionMutex);
                    cond.notify_all(); // Notify the print thread
                }
                // only if mostGraphConnected
                if(mostGraphConnected){
                     //   Starts a new thread (printThread) asynchronously to perform additional tasks such as printing.
                    std::thread printThread(&proactor::printThread, this);
                    printThread.detach(); // Detach the thread to run asynchronously
                }

                // Update wasMajority based on current conditions
                if (notLongerMajority )
                {
                    std::lock_guard<std::mutex> lock(conditionMutex);
                    std::cout << "\nAt least 50% of the graph NO LONGER belongs to the same SCC\n";
                    notLongerMajority = false;
                }
                
                // Respond to client
                std::stringstream response;
                response << "\nKosaraju command executed.\nThe SCC's are:\n";
                for (const auto &scc : sccs)
                {
                    for (int node : scc)
                    {
                        response << node << " ";
                    }
                    response << std::endl; // Print each SCC on a new line
                }
                send(clientSocket, response.str().c_str(), response.str().length(), 0);
            }
            else if (command == "Newedge")
            {
                int i, j;
                ss >> i >> j;
                std::cout << "[Server] Newedge command: adding edge " << i << " -> " << j << std::endl;

                // Lock graph for modification
                std::lock_guard<std::mutex> lock(graphMutex);

                graph.addEdge(i, j);
                send(clientSocket, "Newedge command executed successfully.\n", strlen("Newedge command executed successfully.\n"), 0);
            }
            else if (command == "Removeedge")
            {
                int i, j;
                ss >> i >> j;
                std::cout << "[Server] Removeedge command: Removing edge " << i << " -> " << j << std::endl;

                // Lock graph for modification
                std::lock_guard<std::mutex> lock(graphMutex);

                graph.removeEdge(i, j);
                send(clientSocket, "Removeedge command executed successfully.\n", strlen("Removeedge command executed successfully.\n"), 0);
            }
            else if (command == "Quit")
            {
                std::cout << "[Server] Client requested to quit. Closing connection." << std::endl;
                close(clientSocket);
                return;
            }
            else if (command == "Exit")
            {
                break; // Exit the loop if command is "Exit"
            }
            else if (bytesRead == 0)
            {
                // Client disconnected
                std::cout << "[Server] Client disconnected" << std::endl;
                break;
            }
            

            // Debug print current state of the graph
            {
                std::lock_guard<std::mutex> lock(graphMutex);
                graph.printGraph(); // Make sure you have a printGraph() method to visualize the graph state
            }
            std::cout << std::endl; // Add extra line for clarity
        }
        else if (bytesRead == 0)
        {
            std::cerr << "[Server] Client disconnected. Closing connection." << std::endl;
            close(clientSocket);
            return;
        }
        else
            {
                // Handle invalid command or end of input
                send(clientSocket, "Invalid command.\n", strlen("Invalid command.\n"), 0);
            }
       
    }
}
