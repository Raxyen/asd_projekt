#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <climits>
#include <unordered_set>

using namespace std;

class Vertex {// jako wierzcholek
public:
    string global_id;
    int x, y;
    int type;
    int spec;

    Vertex(string id, int x, int y, int spec)
        : global_id(id), x(x), y(y), spec(spec) {
        type = global_id.back() - '0'; // typ to ostatnia cyfra ID
    }
};

class Edge {
public:
    int from, to;
    int capacity;
    int flow;
    int rev; // indeks krawędzi odwrotnej (w liście sąsiedztwa wierzchołka 'to')
    int cost; // koszt naprawy drogi (lub koszt przepływu przez krawędź)
    
    Edge(int f, int t, int c, int fl, int r, int cost = 0)
        : from(f), to(t), capacity(c), flow(fl), rev(r), cost(cost) {}
};

class Graph {
public:
    unordered_map<string, int> idToIndex; // mapuje ID (np. 0064) na indeks w wektorze
    vector<string> indexToId; // odwrotna mapa (indeks → ID)
    vector<Vertex> vertices;
    vector<vector<Edge>> adjList;  // lista sąsiedztwa

    void addVertex(const Vertex& v) {
        if (idToIndex.count(v.global_id)) return; // unikaj duplikatów
        idToIndex[v.global_id] = vertices.size();
        indexToId.push_back(v.global_id);
        vertices.push_back(v);
        adjList.emplace_back();  // dodaj pustą listę sąsiedztwa
    }

    void addEdgeById(const string& fromId, const string& toId, int capacity, int cost = 0) {
        int u = idToIndex[fromId];
        int v = idToIndex[toId];
        adjList[u].emplace_back(u, v, capacity, 0, adjList[v].size(), cost);
        adjList[v].emplace_back(v, u, 0, 0, adjList[u].size() - 1, -cost); // koszt odwrotny = -cost
    }

    void splitVertices() {
        vector<int> toSplitIdx;
        for (int i = 0; i < vertices.size(); ++i) {
            if (vertices[i].type == 3 || vertices[i].type == 4) {
                toSplitIdx.push_back(i);
            }
        }
        struct Info { int idx; string id; int x, y, spec; };
        vector<Info> infos;
        infos.reserve(toSplitIdx.size());
        for (int idx : toSplitIdx) {
            const Vertex& v = vertices[idx];
            infos.push_back({ idx, v.global_id, v.x, v.y, v.spec });
        }
        for (const auto& info : infos) {
            string inId = info.id + "In";
            string outId = info.id + "Out";
            addVertex(Vertex(inId, info.x, info.y, info.spec));
            addVertex(Vertex(outId, info.x, info.y, info.spec));
            vector<tuple<string, int, int>> incoming, outgoing;
            for (int u = 0; u < adjList.size(); ++u) {
                for (auto& e : adjList[u]) {
                    if (e.to == info.idx && e.capacity > 0) {
                        incoming.emplace_back(indexToId[u], e.capacity, e.cost);
                    }
                }
            }
            for (auto& e : adjList[info.idx]) {
                if (e.capacity > 0) {
                    outgoing.emplace_back(indexToId[e.to], e.capacity, e.cost);
                }
            }
            addEdgeById(inId, outId, info.spec);
            for (auto& [fromId, cap, cost] : incoming)
                addEdgeById(fromId, inId, cap, cost);

            for (auto& [toId, cap, cost] : outgoing)
                addEdgeById(outId, toId, cap, cost);
            // Zero out all capacities on original vertex
            for (auto& e : adjList[info.idx]) {
                e.capacity = 0;
            }
            for (int u = 0; u < adjList.size(); ++u) {
                for (auto& e : adjList[u]) {
                    if (e.to == info.idx) {
                        e.capacity = 0;
                    }
                }
            }
        }
    }
};

void loadVertices(Graph& g, const string& filename) {
    ifstream in(filename);
    if (!in) {
        cerr << "Nie można otworzyć pliku z wierzchołkami: " << filename << endl;
        return;
    }

    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        int x, y, spec;
        string id;
        ss >> x >> y >> id >> spec;
        g.addVertex(Vertex(id, x, y, spec));
    }
}

void loadEdges(Graph& g, const string& filename) { // ładowanie dróg (krawędzi grafu) z pliku
    ifstream in(filename);
    if (!in) {
        cerr << "Nie można otworzyć pliku z drogami: " << filename << endl;
        return;
    }

    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        string fromId, toId;
        int capacity, cost;
        ss >> fromId >> toId >> capacity >> cost; // skąd, dokąd, przepływ, koszt naprawy
        if (g.idToIndex.count(fromId) == 0 || g.idToIndex.count(toId) == 0) {
            cerr << "Uwaga: jeden z wierzchołków nie istnieje: " << fromId << " lub " << toId << endl;
            continue;
        }
        g.addEdgeById(fromId, toId, capacity, cost);
    }
}

int bfs(Graph& g, int s, int t, vector<int>& parent, vector<int>& parentEdgeIndex) {
    fill(parent.begin(), parent.end(), -1);
    queue<int> q;
    q.push(s);
    parent[s] = s;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int i = 0; i < g.adjList[u].size(); i++) {
            Edge& e = g.adjList[u][i];
            if (parent[e.to] == -1 && e.capacity > e.flow) {
                parent[e.to] = u;
                parentEdgeIndex[e.to] = i;
                if (e.to == t) return true;
                q.push(e.to);
            }
        }
    }
    return false;
}

int edmondsKarp(Graph& g, const string& sourceId, const string& sinkId) {
    int s = g.idToIndex[sourceId];// indeks źródła
    int t = g.idToIndex[sinkId];// indeks ujścia
    int maxFlow = 0;

    vector<int> parent(g.vertices.size()); // przechowuje ścieżkę
    vector<int> parentEdgeIndex(g.vertices.size());  //indeksy krawędzi prowadzących do wierzchołków

    while (bfs(g, s, t, parent, parentEdgeIndex)) {
        int pathFlow = INT_MAX;

        // Znajdujemy minimalną przepustowość na ścieżce
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            Edge& e = g.adjList[u][parentEdgeIndex[v]];
            pathFlow = min(pathFlow, e.capacity - e.flow);
        }
        // Aktualizujemy przepływy na ścieżce
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            Edge& e = g.adjList[u][parentEdgeIndex[v]];
            e.flow += pathFlow;// dodajemy przepływ
            g.adjList[e.to][e.rev].flow -= pathFlow; // odejmujemy w odwrotnej krawędzi
        }
        maxFlow += pathFlow;
    }
    return maxFlow;
}

int minCostFlow(Graph& g, const string& sourceId, const string& sinkId, int requiredFlow, int& totalCost) {
    int n = g.vertices.size();
    int s = g.idToIndex[sourceId];
    int t = g.idToIndex[sinkId];
    vector<int> potential(n, 0);
    totalCost = 0;
    int flow = 0;

    while (flow < requiredFlow) {
        vector<int> dist(n, INT_MAX);
        vector<int> parent(n, -1);
        vector<int> parentEdgeIndex(n, -1);
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq;

        dist[s] = 0;
        pq.emplace(0, s);

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;

            for (int i = 0; i < g.adjList[u].size(); ++i) {
                Edge& e = g.adjList[u][i];
                if (e.capacity > e.flow) {
                    int v = e.to;
                    int newDist = dist[u] + e.cost + potential[u] - potential[v];
                    if (newDist < dist[v]) {
                        dist[v] = newDist;
                        parent[v] = u;
                        parentEdgeIndex[v] = i;
                        pq.emplace(dist[v], v);
                    }
                }
            }
        }

        if (parent[t] == -1) break; // nie da się już nic przepchnąć

        for (int i = 0; i < n; ++i)
            if (dist[i] < INT_MAX)
                potential[i] += dist[i];

        int pathFlow = requiredFlow - flow;
        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            Edge& e = g.adjList[u][parentEdgeIndex[v]];
            pathFlow = min(pathFlow, e.capacity - e.flow);
        }

        for (int v = t; v != s; v = parent[v]) {
            int u = parent[v];
            Edge& e = g.adjList[u][parentEdgeIndex[v]];
            e.flow += pathFlow;
            g.adjList[e.to][e.rev].flow -= pathFlow;
            totalCost += pathFlow * e.cost;
        }

        flow += pathFlow;
    }
    return flow;
}

int main() {
    Graph g;

    // Wczytanie danych
    loadVertices(g, "struktury.txt");
    loadEdges(g, "drogi.txt");

    // Najpierw splitujemy pola i browary
    g.splitVertices();

    // Dodajemy sztuczne źródło i ujście
    g.addVertex(Vertex("SOURCE", 0, 0, 0));
    g.addVertex(Vertex("SINK", 0, 0, 0));

    // Połączenia z/dla SOURCE i SINK
    for (const Vertex& v : g.vertices) {
        if (v.global_id == "SOURCE" || v.global_id == "SINK") continue;

        if (v.global_id.ends_with("In") && v.global_id.substr(0, 4).back() == '3') {
            // Pole: SOURCE → PoleIn
            g.addEdgeById("SOURCE", v.global_id, v.spec);
        }
        else if (v.global_id.substr(0, 4).back() == '2') {
            // Karczma: Karczma → SINK
            g.addEdgeById(v.global_id, "SINK", INT_MAX);
        }
    }

    // Wyświetlenie (opcjonalne)
    cout << "=== Wierzcholki ===" << endl;
    for (const auto& v : g.vertices)
        cout << v.global_id << " (" << v.x << "," << v.y << "), typ: " << v.type << ", spec: " << v.spec << endl;

    cout << "\n=== Krawedzie ===" << endl;
    for (int i = 0; i < g.adjList.size(); ++i) {
        string fromId = g.indexToId[i];
        for (const Edge& e : g.adjList[i]) {
            string toId = g.indexToId[e.to];
            cout << fromId << " -> " << toId << " | cap: " << e.capacity << ", flow: " << e.flow << ", cost: " << e.cost << endl;
        }
    }

    // Edmonds-Karp: obliczenie maksymalnego przepływu
    int maxFlow = edmondsKarp(g, "SOURCE", "SINK");
    cout << "\nMaksymalny przeplyw (bez kosztow): " << maxFlow << endl;

    // Reset przepływu
    for (auto& edges : g.adjList)
        for (auto& e : edges)
            e.flow = 0;

    // Min-cost flow dla tej samej wartości
    int totalRepairCost = 0;
    int achievedFlow = minCostFlow(g, "SOURCE", "SINK", maxFlow, totalRepairCost);

    cout << "Koszt naprawy drog potrzebnych do osiagniecia przeplywu: " << totalRepairCost << endl;
    return 0;
}
