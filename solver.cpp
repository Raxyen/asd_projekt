#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

class Vertex {
public:
    string global_id;
    int x, y;
    int type;
    int spec;

    Vertex(string id, int x, int y, int spec)
        : global_id(id), x(x), y(y), spec(spec) {
        type = global_id.back() - '0';
    }
};

class Edge {
public:
    int from, to;
    int capacity;
    int flow;
    int rev; // indeks krawedzi powrotnej;

    Edge(int f, int t, int c, int fl, int r) : from(f), to(t), capacity(c), flow(fl), rev(r) {}
};

class Graph {
public:
    unordered_map<string, int> idToIndex;
    vector<string> indexToId;
    vector<Vertex> vertices;
    vector<vector<Edge>> adjList;

    void addVertex(Vertex v) {
        idToIndex[v.global_id] = vertices.size();
        indexToId.push_back(v.global_id);
        vertices.push_back(v);
        adjList.emplace_back();
    }

    void addEdgeById(const string& fromId, const string& toId, int capacity) {
        int u = idToIndex[fromId];
        int v = idToIndex[toId];
        adjList[u].emplace_back(u, v, capacity, 0, adjList[v].size());
        adjList[v].emplace_back(v, u, 0, 0, adjList[u].size() - 1); 

    }
};

void loadVertices(Graph& g, const string& filename) {
    ifstream in(filename);
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        int x, y, spec;
        string id;
        ss >> x >> y >> id >> spec;
        g.addVertex(Vertex(id, x, y, spec));
    }
}

int main() {
    
    return 0;
}
