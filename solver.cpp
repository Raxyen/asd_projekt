#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>

using namespace std;

// Klasa reprezentująca wierzchołek (np. pole, browar, karczma, skrzyżowanie)
class Vertex {
public:
    string global_id; // unikalny identyfikator (np. "0064")
    int x, y;         // współrzędne geograficzne
    int type;         // typ obiektu (1=browar, 2=karczma, 3=pole, 4=skrzyżowanie)
    int spec;         // dodatkowa wartość (np. wydajność, pojemność itd.)

    // Konstruktor – automatycznie rozpoznaje typ na podstawie ostatniej cyfry ID
    Vertex(string id, int x, int y, int spec)
        : global_id(id), x(x), y(y), spec(spec) {
        type = global_id.back() - '0';
    }
};

// Klasa reprezentująca skierowaną krawędź w grafie przepływowym
class Edge {
public:
    int from;     // indeks wierzchołka początkowego
    int to;       // indeks wierzchołka końcowego
    int capacity; // maksymalna przepustowość (np. ile ton można przewieźć)
    int flow;     // aktualny przepływ (ile aktualnie przepływa)
    int rev;      // indeks krawędzi odwrotnej w liście sąsiedztwa wierzchołka 'to'

    // Konstruktor
    Edge(int f, int t, int c, int fl, int r) : from(f), to(t), capacity(c), flow(fl), rev(r) {}
};

// Klasa reprezentująca cały graf przepływowy
class Graph {
public:
    unordered_map<string, int> idToIndex; // mapa: ID obiektu → indeks w wektorach grafu
    vector<string> indexToId;             // odwrotna mapa: indeks → ID obiektu
    vector<Vertex> vertices;              // lista wierzchołków
    vector<vector<Edge>> adjList;         // lista sąsiedztwa (dla każdego wierzchołka)

    // Dodaje nowy wierzchołek do grafu
    void addVertex(Vertex v) {
        idToIndex[v.global_id] = vertices.size(); // przypisz numer indeksu
        indexToId.push_back(v.global_id);
        vertices.push_back(v);
        adjList.emplace_back(); // dodaj pustą listę sąsiadów
    }

    // Dodaje skierowaną krawędź (i krawędź odwrotną) na podstawie ID tekstowych
    void addEdgeById(const string& fromId, const string& toId, int capacity) {
        int u = idToIndex[fromId]; // indeks źródła
        int v = idToIndex[toId];   // indeks celu
        // Dodaj krawędź główną
        adjList[u].emplace_back(u, v, capacity, 0, adjList[v].size());
        // Dodaj krawędź odwrotną o przepustowości 0
        adjList[v].emplace_back(v, u, 0, 0, adjList[u].size() - 1);
    }
};

// Funkcja do wczytywania wierzchołków z pliku
void loadVertices(Graph& g, const string& filename) {
    ifstream in(filename);
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        int x, y, spec;
        string id;
        ss >> x >> y >> id >> spec; // format pliku: X Y ID SPEC
        g.addVertex(Vertex(id, x, y, spec));
    }
}

// Funkcja do wczytywania dróg (krawędzi) z pliku
void loadEdges(Graph& g, const string& filename) {
    ifstream in(filename);
    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        string fromId, toId;
        int capacity;
        ss >> fromId >> toId >> capacity; // format: ID_FROM ID_TO PRZEPUSTOWOŚĆ
        g.addEdgeById(fromId, toId, capacity);
    }
}

int main() {
    Graph g;

    // Wczytaj dane z plików
    loadVertices(g, "D:\\DataUser\\Downloads\\struktury.txt"); // wierzchołki
    loadEdges(g, "D:\\DataUser\\Downloads\\drogi.txt");         // krawędzie

    // WYŚWIETLENIE – wizualizacja listy sąsiedztwa
    for (int i = 0; i < g.vertices.size(); i++) {
        cout << g.indexToId[i] << " (" << g.vertices[i].x << "," << g.vertices[i].y << ") -> ";
        for (const auto& edge : g.adjList[i]) {
            cout << g.indexToId[edge.to] << "(cap:" << edge.capacity << ") ";
        }
        cout << endl;
    }

    // WYŚWIETLENIE – pełna lista wierzchołków z typem i spec
    cout << "=== Wierzchołki ===" << endl;
    for (const auto& v : g.vertices) {
        cout << v.global_id << " (" << v.x << "," << v.y << "), typ: " << v.type << ", spec: " << v.spec << endl;
    }

    // WYŚWIETLENIE – wszystkie krawędzie (razem z odwrotnymi)
    cout << "\n=== Krawędzie ===" << endl;
    for (int i = 0; i < g.adjList.size(); ++i) {
        string fromId = g.indexToId[i];
        for (const Edge& e : g.adjList[i]) {
            string toId = g.indexToId[e.to];
            cout << fromId << " -> " << toId << " | przepustowość: " << e.capacity << ", flow: " << e.flow << endl;
        }
    }

    return 0;
}
