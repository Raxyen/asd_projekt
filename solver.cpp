#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <climits>
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

    Edge(int f, int t, int c, int fl, int r) : from(f), to(t), capacity(c), flow(fl), rev(r) {}
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

    void addEdgeById(const string& fromId, const string& toId, int capacity) {
        int u = idToIndex[fromId];
        int v = idToIndex[toId];
        adjList[u].emplace_back(u, v, capacity, 0, adjList[v].size());
        adjList[v].emplace_back(v, u, 0, 0, adjList[u].size() - 1); // krawędź odwrotna
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

void loadEdges(Graph& g, const string& filename) {
    ifstream in(filename);
    if (!in) {
        cerr << "Nie można otworzyć pliku z drogami: " << filename << endl;
        return;
    }

    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        string fromId, toId;
        int capacity;
        ss >> fromId >> toId >> capacity;

        if (g.idToIndex.count(fromId) == 0 || g.idToIndex.count(toId) == 0) {
            cerr << "Uwaga: jeden z wierzchołków nie istnieje: " << fromId << " lub " << toId << endl;
            continue;
        }

        g.addEdgeById(fromId, toId, capacity);
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
//тут правильно 
int main() {
    Graph g;

    // Podaj ścieżki do plików z danymi
    loadVertices(g, "D:\\DataUser\\Downloads\\struktury.txt");
    loadEdges(g, "D:\\DataUser\\Downloads\\drogi.txt");

    g.addVertex(Vertex("SOURCE", 0, 0, 0));
    g.addVertex(Vertex("SINK", 0, 0, 0));

    for (const Vertex& v : g.vertices) {
        if (v.global_id == "SOURCE" || v.global_id == "SINK") continue;

        if (v.type == 3) { // pole 
            g.addEdgeById("SOURCE", v.global_id, v.spec); // spec = сколько производит
        }
        else if (v.type == 2) { // karczma 
            g.addEdgeById(v.global_id, "SINK", INT_MAX); // можно ограничить если надо
        }
    }



    // Sprawdzenie: wypisz wszystkie wierzchołki i ich sąsiadów
    for (int i = 0; i < g.vertices.size(); i++) {
        cout << g.indexToId[i] << " (" << g.vertices[i].x << "," << g.vertices[i].y << ") -> ";
        for (const auto& edge : g.adjList[i]) {
            cout << g.indexToId[edge.to] << "(cap:" << edge.capacity << ") ";
        }
        cout << endl;
    }

    cout << "=== Wierzcholki ===" << endl;
    for (const auto& v : g.vertices) {
        cout << v.global_id << " (" << v.x << "," << v.y << "), typ: " << v.type << ", spec: " << v.spec << endl;
    }

    // Informacja o wszystkich krawędziach (przefiltrowana żeby nie dublować odwrotnych)
    cout << "\n=== Krawedzie ===" << endl;
    for (int i = 0; i < g.adjList.size(); ++i) {
        string fromId = g.indexToId[i];
        for (const Edge& e : g.adjList[i]) {
            string toId = g.indexToId[e.to];
            cout << fromId << " -> " << toId << " | przepustowosc: " << e.capacity << ", flow: " << e.flow << endl;
        }
    }


    cout << "Maksymalny przeplyw: " << edmondsKarp(g, "SOURCE", "SINK") << endl;
   // cout << "Maksymalny przeplyw: " << edmondsKarp(g, "0003", "0052") << endl;





    return 0;
}
