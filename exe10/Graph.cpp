#include "Graph.hpp"
#include <vector>  // Include the vector header
#include <iostream> 

using namespace std; 

Graph::Graph(){}
Graph::Graph(int n) {
    this->n = n;
    adj.assign(n + 1, std::vector<int>(n + 1, 0)); // Initialize adjacency matrix with size (n+1)x(n+1)
}

void Graph::addEdge(int u, int v) {
    adj[u][v] = 1; // Assuming it's a directed graph
    std::cout << "New edge added between " << u << " and " << v << std::endl;
}

int Graph::getNumVertices() const {
    return n;
}

void Graph::removeEdge(int u, int v) {
    if (u >= 1 && u <= n && v >= 1 && v <= n) { // Ensure indices are within bounds
        adj[u][v] = 0;
        std::cout << "Removed the edge between " << u << " and " << v << std::endl;
    } else {
        std::cerr << "Invalid indices for removeEdge: " << u << ", " << v << std::endl;
    }
}


void Graph::printGraph() const {
    std::cout << "Current Graph:" << std::endl;
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            std::cout << adj[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void Graph::dfs(int v, std::vector<bool>& visited, std::stack<int>& st) {
    visited[v] = true;
    for (int u = 1; u <= n; ++u) {
        if (adj[v][u] && !visited[u]) {
            dfs(u, visited, st);
        }
    }
    st.push(v);
}

void Graph::dfsTranspose(int v, std::vector<bool>& visited, std::vector<int>& component, const Graph& transposedGraph) {
    visited[v] = true;
    component.push_back(v);
    for (int u = 1; u <= n; ++u) {
        if (transposedGraph.adj[v][u] && !visited[u]) {
            dfsTranspose(u, visited, component, transposedGraph);
        }
    }
}

Graph Graph::getTranspose() const {
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

vector<std::vector<int>> Graph::kosaraju() {
    std::vector<bool> visited(n + 1, false);
    std::stack<int> st;

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
    std::vector<std::vector<int>> sccs;

    // Step 3: Process nodes in order defined by stack
    while (!st.empty()) {
        int v = st.top();
        st.pop();
        if (!visited[v]) {
            std::vector<int> component;
            transposedGraph.dfsTranspose(v, visited, component, transposedGraph);
            sccs.push_back(component);
        }
    }

    return sccs;
}

// Method to clear the graph
void Graph::clear() {
    for (int i = 0; i < n; ++i) {
        adj[i].clear();
    }
}

// Method to resize the graph
void Graph::resize(int V) {
    this->n = n;
    adj.resize(n);
}