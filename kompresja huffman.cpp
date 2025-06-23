#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <string>

using namespace std;

struct Node {
    char ch;
    int freq;
    Node* left;
    Node* right;
    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

Node* buildHuffmanTree(const unordered_map<char, int>& freq) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }

    if (pq.empty()) return nullptr;

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }

    return pq.top();
}

void generateCodes(Node* root, const string& code, unordered_map<char, string>& huffmanCode) {
    if (!root) return;
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = code;
    }
    generateCodes(root->left, code + "0", huffmanCode);
    generateCodes(root->right, code + "1", huffmanCode);
}

string compress(const vector<char>& data, unordered_map<char, string>& huffmanCode) {
    string compressedData;
    for (char ch : data) {
        compressedData += huffmanCode[ch];
    }
    return compressedData;
}

void writeToFile(const string& compressedData, const string& filename) {
    ofstream outFile(filename, ios::binary);
    bitset<8> bits;
    for (size_t i = 0; i < compressedData.size(); i++) {
        bits[i % 8] = compressedData[i] - '0';
        if (i % 8 == 7 || i == compressedData.size() - 1) {
            outFile.put(static_cast<unsigned char>(bits.to_ulong()));
            bits.reset();
        }
    }
    outFile.close();
}

string readFromFile(const string& filename) {
    ifstream inFile(filename, ios::binary);
    string compressedData;
    bitset<8> bits;
    char byte;
    while (inFile.get(byte)) {
        bits = bitset<8>(static_cast<unsigned char>(byte));
        for (int i = 0; i < 8; i++) {
            compressedData += bits[i] ? '1' : '0';
        }
    }
    inFile.close();
    return compressedData;
}

vector<char> decompress(Node* root, const string& compressedData) {
    vector<char> result;
    Node* curr = root;
    for (char bit : compressedData) {
        if (!curr) break;
        curr = (bit == '0') ? curr->left : curr->right;
        if (curr && !curr->left && !curr->right) {
            result.push_back(curr->ch);
            curr = root;
        }
    }
    return result;
}

void saveCodes(const unordered_map<char, string>& huffmanCode, const string& filename) {
    ofstream out(filename);
    for (const auto& pair : huffmanCode) {
        out << static_cast<int>(static_cast<unsigned char>(pair.first)) << ' ' << pair.second << '\n';
    }
    out.close();
}

unordered_map<char, string> loadCodes(const string& filename) {
    unordered_map<char, string> codes;
    ifstream in(filename);
    int ascii;
    string code;
    while (in >> ascii >> code) {
        codes[static_cast<char>(ascii)] = code;
    }
    in.close();
    return codes;
}

Node* rebuildTree(const unordered_map<char, string>& codes) {
    Node* root = new Node('\0', 0);
    for (const auto& pair : codes) {
        Node* curr = root;
        for (char bit : pair.second) {
            if (bit == '0') {
                if (!curr->left) curr->left = new Node('\0', 0);
                curr = curr->left;
            }
            else {
                if (!curr->right) curr->right = new Node('\0', 0);
                curr = curr->right;
            }
        }
        curr->ch = pair.first;
    }
    return root;
}

int main() {
    string in_file_name, out_file_name;
    cout << "------MENU------\n1 | Kompresja\n2 | Dekompresja\n";
    int mode; 
    cin >> mode;
    cout << "Podaj nazwę pliku wejsciowego: "; 
    cin >> in_file_name;
    cout << "Podaj nazwę pliku wyjsciowego: "; 
    cin >> out_file_name;

    if (mode == 1) {
        ifstream inFile(in_file_name, ios::binary);
        vector<char> data((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        inFile.close();

        if (data.empty()) {
            cerr << "Plik jest pusty! Brak danych do kompresji.\n";
            return 1;
        }

        unordered_map<char, int> freq;
        for (char ch : data) freq[ch]++;
        Node* root = buildHuffmanTree(freq);
        unordered_map<char, string> huffmanCode;
        generateCodes(root, "", huffmanCode);

        string compressedData = compress(data, huffmanCode);
        writeToFile(compressedData, out_file_name);
        saveCodes(huffmanCode, "huffman_codes.txt");

        cout << "Plik skompresowany jako: " << out_file_name << "\nSłownik zapisany w: huffman_codes.txt\n";
    }

    else if (mode == 2) {
        string compressed = readFromFile(in_file_name);
        auto codes = loadCodes("huffman_codes.txt");

        if (codes.empty()) {
            cerr << "Brak słownika lub pusty! Nie można zdekompresować.\n";
            return 1;
        }

        Node* root = rebuildTree(codes);
        vector<char> decompressed = decompress(root, compressed);
        ofstream out(out_file_name, ios::binary);
        out.write(decompressed.data(), decompressed.size());
        out.close();
        cout << "Zdekompresowano do: " << out_file_name << '\n';
    }

    else {
        cout << "Nieprawidłowy tryb!\n";
    }

    return 0;
}
