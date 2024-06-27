#include <iostream>
#include <vector>
#include <list>

using namespace std;

class Graph {
private:
    int n; // number of nodes
    vector<list<int>> adj; // adjacency list

public:
    Graph(int n) {
        this->n = n;
        adj.resize(n + 1);
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }

    void dfs(int v, vector<bool>& visited, vector<int>& finishTimes) {
        visited[v] = true;
        for (int u : adj[v]) {
            if (!visited[u]) {
                dfs(u, visited, finishTimes);
            }
        }
        finishTimes.push_back(v);
    }

    void dfsTranspose(int v, vector<bool>& visited, vector<int>& component, const vector<vector<int>>& adjMatrixT) {
        visited[v] = true;
        component.push_back(v);
        for (int u = 1; u <= n; ++u) {
            if (adjMatrixT[v][u] && !visited[u]) {
                dfsTranspose(u, visited, component, adjMatrixT);
            }
        }
    }

    vector<vector<int>> kosaraju() {
        vector<bool> visited(n + 1, false);
        vector<int> finishTimes;

        // Step 1: Perform DFS and record finishing times
        for (int i = 1; i <= n; ++i) {
            if (!visited[i]) {
                dfs(i, visited, finishTimes);
            }
        }

        // Initialize adjacency matrix from adjacency list
        vector<vector<int>> adjMatrix(n + 1, vector<int>(n + 1, 0));
        for (int u = 1; u <= n; ++u) {
            for (int v : adj[u]) {
                adjMatrix[u][v] = 1;
            }
        }

        // Transpose the graph using the adjacency matrix
        vector<vector<int>> adjMatrixT(n + 1, vector<int>(n + 1, 0));
        for (int u = 1; u <= n; ++u) {
            for (int v = 1; v <= n; ++v) {
                adjMatrixT[u][v] = adjMatrix[v][u];
            }
        }

        // Clear visited array for reuse in second DFS
        visited.assign(n + 1, false);
        vector<vector<int>> sccs;

        // Step 3: Process nodes in order defined by finishing times
        for (int i = finishTimes.size() - 1; i >= 0; --i) {
            int v = finishTimes[i];
            if (!visited[v]) {
                vector<int> component;
                dfsTranspose(v, visited, component, adjMatrixT);
                sccs.push_back(component);
            }
        }

        return sccs;
    }
};

int main() {
    int n, m;
    cout << "Enter the number of vertices (n) and edges (m): ";
    cin >> n >> m;

    Graph graph(n);

    cout << "Enter the edges (u v):" << endl;
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        graph.addEdge(u, v);
    }

    // Find strongly connected components (SCCs) using Kosaraju's algorithm
    vector<vector<int>> sccs = graph.kosaraju();

    cout << "The SCC's are:" << endl;

    // Output SCCs
    for (const auto& scc : sccs) {
        for (int node : scc) {
            cout << node  << " ";
        }
        cout << "\n";
    }

    return 0;
}
   