#include <iostream>
#include <vector>
#include <sstream>
#include <stack>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <poll.h>

using namespace std;

class Graph {
        private:
        int n; // number of nodes
        vector<vector<int>> adj; // adjacency matrix

        public:
        Graph(int n) {
            this->n = n;
            adj.assign(n + 1, vector<int>(n + 1, 0)); // Initialize adjacency matrix with size (n+1)x(n+1)
        }

        void addEdge(int u, int v) {
            adj[u][v] = 1; //  directed graph
            cout <<"new edge added between  "<< u << " and " << v << endl;
        }

    void removeEdge(int u, int v) {
        adj[u][v] = 0;
        cout <<"removed the edge between  "<< u << " and " << v << endl;
    }

    void printGraph() const {
        cout << "Current Graph:" << endl;
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= n; ++j) {
                cout << adj[i][j] << " ";
            }
            cout << endl;
        }
    }
        void dfs(int v, vector<bool>& visited, stack<int>& st) {
            visited[v] = true;
            for (int u = 1; u <= n; ++u) {
                if (adj[v][u] && !visited[u]) {
                    dfs(u, visited, st);
                }
            }
            st.push(v);
        }

        void dfsTranspose(int v, vector<bool>& visited, vector<int>& component, const Graph& transposedGraph) {
            visited[v] = true;
            component.push_back(v);
            for (int u = 1; u <= n; ++u) {
                if (transposedGraph.adj[v][u] && !visited[u]) {
                    dfsTranspose(u, visited, component, transposedGraph);
                }
            }
        }

        Graph getTranspose() const {
            Graph transposedGraph(n);
            for (int u = 1; u <= n; ++u) {
                for (int v = 1; v <= n; ++v) {
                    if (adj[u][v]) {
                        transposedGraph.addEdge(v, u);
                    }
                }
            }
            return transposedGraph;
        }

        vector<vector<int>> kosaraju() {
            vector<bool> visited(n + 1, false);
            stack<int> st;

            // Step 1: Perform DFS and fill stack with finishing times
            for (int i = 1; i <= n; ++i) {
                if (!visited[i]) {
                    dfs(i, visited, st);
                }
            }

            // Step 2: Transpose the graph
            Graph transposedGraph = getTranspose();

            // Clear visited array for reuse in second DFS
            visited.assign(n + 1, false);
            vector<vector<int>> sccs;

            // Step 3: Process nodes in order defined by stack
            while (!st.empty()) {
                int v = st.top();
                st.pop();
                if (!visited[v]) {
                    vector<int> component;
                    transposedGraph.dfsTranspose(v, visited, component, transposedGraph);
                    sccs.push_back(component);
                }
            }

            return sccs;
        }


};

void handleClientCommands(int clientSocket, Graph& graph) {
    char buffer[1024] = {0}; //init the buffer
    int valread = read(clientSocket, buffer, 1024); // read data from client socket into buffer
    if (valread <= 0) {
        if (valread == 0) {
            cout << "Client disconnected." << endl;
        } 
        else {
            perror("recv");
        }
    close(clientSocket);
    return;
    }

    stringstream ss(buffer);//stringstream to parse the command from the buffer
    string command; //define the command that given by the client
    ss >> command;

    if (command == "Newgraph") {
        int n, m;
        ss >> n >> m;
        graph = Graph(n); // Reinitialize graph

        for (int i = 0; i < m; ++i) {
            memset(buffer, 0, sizeof(buffer));// clear the buffer
            valread = read(clientSocket, buffer, 1024);
            if (valread == 0) {
                cout << "Client disconnected while sending graph edges." << endl;
                close(clientSocket);
                return;
            }
            stringstream edgeStream(buffer);//take edges from the client
            int u, v;
            edgeStream >> u >> v;
            graph.addEdge(u, v);
        }
    } 
    else if (command == "Kosaraju") {
        // Command to execute Kosaraju's algorithm to find SCCs
        vector<vector<int>> sccs = graph.kosaraju();
        stringstream response;
        response << "Kosaraju command executed.\nThe SCC's are:\n";
            for (const auto& scc : sccs) {
                for (int node : scc) {
                    response << node << " ";
                }
                response << endl; // Print each SCC on a new line
            }
        //send the SCCs to the client    
        send(clientSocket, response.str().c_str(), response.str().length(), 0);
    } 
    else if (command == "Newedge") {
        // Command to add a new edge to the graph
        int i, j;
        ss >> i >> j;
        cout << "Newedge command: adding edge " << i << " -> " << j << endl;
        graph.addEdge(i, j);

        send(clientSocket, "Newedge command executed successfully.\n", strlen("Newedge command executed successfully.\n"), 0);
    } 
    else if (command == "Removeedge") {
        // Command to remove an edge from the graph
        int i, j;
        ss >> i >> j;
        cout << "Removeedge command: Removing edge " << i << " -> " << j << endl;
        graph.removeEdge(i, j);
        send(clientSocket, "Removeedge command executed successfully.\n", strlen("Removeedge command executed successfully.\n"), 0);
    } 
    else {
        // Handle invalid command or end of input
        send(clientSocket, "Invalid command.\n", strlen("Invalid command.\n"), 0);
        }

        //print current state of the graph
    graph.printGraph(); 
    cout << endl; //Yeridat sora
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
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Initialize pollfd array
    struct pollfd fds[200]; //Array to hold file descriptors for poll
    int nfds = 1; //the number of file descriptors
    memset(fds, 0, sizeof(fds));

    // Set up initial listening socket
    fds[0].fd = serverSocket; // Listening socket
    fds[0].events = POLLIN; 


    cout << "server opened and now listening for clients to connect" << endl;


    while (true) {
        // Call poll to wait for events on the file descriptors
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("poll error");
            exit(EXIT_FAILURE);
        }
        //itereate all over the fd
        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                 // Check if there's an event on the listening socket
                if (fds[i].fd == serverSocket) {
                    // Incoming connection
                    int clientSocket;
                    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }

                    cout << "New connection, socket fd is " << clientSocket << ", ip is : " << inet_ntoa(address.sin_addr) << ", port : " << ntohs(address.sin_port) << endl;

                    // add new socket to array of fds
                    fds[nfds].fd = clientSocket;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    // Handle client commands
                    handleClientCommands(fds[i].fd, graph);
                }
            }
        }
    }

    return 0;
}