#include <iostream>
#include <vector>
#include <list>
#include <stack>

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

    void dfs(int v, vector<bool>& visited, stack<int>& st) {
        visited[v] = true;
        for (int u : adj[v]) {
            if (!visited[u]) {
                dfs(u, visited, st);
            }
        }
        st.push(v);
    }

    void dfsTranspose(int v, vector<bool>& visited, vector<int>& component) {
        visited[v] = true;
        component.push_back(v);
        for (int u : adj[v]) {
            if (!visited[u]) {
                dfsTranspose(u, visited, component);
            }
        }
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
        Graph transposedGraph(n);
        for (int u = 1; u <= n; ++u) {
            for (int v : adj[u]) {
                transposedGraph.addEdge(v, u);
            }
        }

        // Clear visited array for reuse in second DFS
        visited.assign(n + 1, false);
        vector<vector<int>> sccs;

        // Step 3: Process nodes in order defined by stack
        while (!st.empty()) {
            int v = st.top();
            st.pop();
            if (!visited[v]) {
                vector<int> component;
                transposedGraph.dfsTranspose(v, visited, component);
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
    
