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

    //dzielimy pola i browary, ale do browarów IN i OUT nie robimy dróg
    void splitVertices() {
        vector<int> toSplitIdx;
        for (int i = 0; i < vertices.size(); ++i) {
            if (vertices[i].type == 3 || vertices[i].type == 1 ) {
                toSplitIdx.push_back(i);
            }
        }
        struct Info { int idx; string id; int x, y, spec, type; };
        vector<Info> infos;
        infos.reserve(toSplitIdx.size());
        for (int idx : toSplitIdx) {
            const Vertex& v = vertices[idx];
            infos.push_back({ idx, v.global_id, v.x, v.y, v.spec, v.type });
        }
        for (const auto& info : infos) {
            string inId = info.id + "In";
            string outId = info.id + "Out";
            addVertex(Vertex(inId, info.x, info.y, info.spec));
            addVertex(Vertex(outId, info.x, info.y, info.spec));
            vector<pair<string, int>> incoming, outgoing;
            //krawędzie wchodzące
            for (int u = 0; u < adjList.size(); ++u) {
                for (auto& e : adjList[u]) {
                    if (e.to == info.idx && e.capacity > 0) {
                        incoming.emplace_back(indexToId[u], e.capacity);
                    }
                }
            }
            // krawędzie wychodzące
            for (auto& e : adjList[info.idx]) {
                if (e.capacity > 0) {
                    outgoing.emplace_back(indexToId[e.to], e.capacity);
                }
            }
            // łączymy pola IN i OUT
            if (info.type == 3) {
                addEdgeById(inId, outId, info.spec);
            }

            for (auto& p : incoming) addEdgeById(p.first, inId, p.second);
            for (auto& p : outgoing) addEdgeById(outId, p.first, p.second);

            // zerujemy capasity pojemności oryginalnej wierzchołka, żeby "skasować drogę"
            for (auto& e : adjList[info.idx]) {
                e.capacity = 0;
            }
            for (auto& edges : adjList) {
                for (auto& e : edges) {
                    if (e.to == info.idx) e.capacity = 0;
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


bool pointOnSegment(int px, int py, int ax, int ay, int bx, int by) {
    int minX = min(ax, bx), maxX = max(ax, bx);
    int minY = min(ay, by), maxY = max(ay, by);
    int cross = (bx - ax)*(py - ay) - (by - ay)*(px - ax);
    if (cross != 0) return false;
    return px >= minX && px <= maxX && py >= minY && py <= maxY;
}

struct Polygon {
    double boost;
    vector<pair<int, int>> points;

    bool contains(int px, int py) const {
        int count = 0;
        int n = points.size();
        for (int i = 0; i < n; ++i) {
            auto [x1, y1] = points[i];
            auto [x2, y2] = points[(i + 1) % n];

            if (px == x1 && py == y1) return true; // wierzchołek
            if (pointOnSegment(px, py, x1, y1, x2, y2)) return true;

            if ((y1 > py) != (y2 > py)) {
                double xinters = (double)(x2 - x1) * (py - y1) / (y2 - y1 + 1e-9) + x1;
                if (px < xinters)
                    count++;
            }
        }
        return count % 2 == 1;
    }
};


vector<Polygon> loadPolygonsFromFile(const string& filename) {
    vector<Polygon> polygons;
    ifstream in(filename);
    if (!in) {
        cerr << "Nie można otworzyć pliku ćwiartek: " << filename << endl;
        return polygons;
    }

    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        double boost;
        ss >> boost;
        vector<pair<int, int>> pts;
        int x, y;
        while (ss >> x >> y) {
            pts.emplace_back(x, y);
        }
        polygons.push_back({boost, pts});
    }
    return polygons;
}

void applyBoostToFields(vector<Vertex>& vertices, const vector<Polygon>& polygons) {
    for (auto& v : vertices) {
        if (v.type == 3) { // Pole uprawne
            for (const auto& poly : polygons) {
                if (poly.contains(v.x, v.y)) {
                    v.spec = static_cast<int>(v.spec * poly.boost);
                    break;
                }
            }
        }
    }
}


//тут правильно
int main() {
    Graph g;

    // Podaj ścieżki do plików z danymi
    loadVertices(g, "D:\\DataUser\\Downloads\\struktury.txt");
    loadEdges(g, "D:\\DataUser\\Downloads\\drogi.txt");


    g.splitVertices();
    vector<Polygon> cwiartki = loadPolygonsFromFile("C:\\Users\\user\\Desktop\\ver30\\cwiartki.txt");
    applyBoostToFields(g.vertices, cwiartki);

    // dodajemy zródłą i ujścia
    g.addVertex(Vertex("SOURCE", 0, 0, 0));
    g.addVertex(Vertex("SINK", 0, 0, 0));
    g.addVertex(Vertex("SOURCEBREWERY", 0, 0, 0));
    g.addVertex(Vertex("SINKBREWERY", 0, 0, 0));

    // łączymy zródło SOURCE z wierzchołkami pol IN
    for (const auto& v : g.vertices) {
        if (v.global_id == "SOURCE" || v.global_id == "SINK" || v.global_id == "SOURCEBREWERY" || v.global_id == "SINKBREWERY") continue;
        // Only consider 'In' vertices for type 3
        if (v.global_id.size() > 2 && v.global_id.substr(v.global_id.size() - 2) == "In" &&
            v.global_id[v.global_id.size() - 3] == '3') {
            g.addEdgeById("SOURCE", v.global_id, v.spec);
        }
    }

    // łączymy ujście SINK z karczmami
    for (const auto& v : g.vertices) {
        if (v.type == 2)
            g.addEdgeById(v.global_id, "SINK", INT_MAX);
    }

    // łączymy browary IN z SOURCEBREWERY i SINKBREWERY z browarami OUT
    for (const auto& v : g.vertices) {
        if (v.global_id.size() > 2 && v.global_id.substr(v.global_id.size() - 2) == "In" &&
            v.global_id[v.global_id.size() - 3] == '1') {
            g.addEdgeById(v.global_id, "SOURCEBREWERY", v.spec);
        }
        if (v.global_id.size() > 3 && v.global_id.substr(v.global_id.size() - 3) == "Out" &&
            v.global_id[v.global_id.size() - 4] == '1') {
            g.addEdgeById("SINKBREWERY", v.global_id, v.spec);
        }
    }

    int maxJeczmien = edmondsKarp(g, "SOURCE", "SOURCEBREWERY"); // maksymalna ilosc jęczmienia, którą można dostarczyć do browarów
    g.addEdgeById("SOURCEBREWERY" , "SINKBREWERY", maxJeczmien/2); // zmiana capasity drogi, bo z 1 kg jęczmienia zyskujemy 0,5 piwa

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


    cout << "Maksymalny przeplyw jeczmienia: " << maxJeczmien << endl;
    cout << "Maksymalny przeplyw piwa (nasz wynik): " << edmondsKarp(g, "SOURCEBREWERY", "SINK") << endl;

    return 0;
}
