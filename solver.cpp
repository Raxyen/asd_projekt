#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <map>

using namespace std;

class Vertex {
private:
	int x, y;
	unsigned int id;
	unsigned int spec;
	char type;
public:
	friend ostream& operator<<(ostream& out, Vertex& v);

	Vertex() {}

	Vertex(int p_x, int p_y, unsigned int p_id, unsigned int p_spec) : x(p_x), y(p_y), id(p_id), spec(p_spec) {
		switch (id) {

		}
	}

	inline void setType(char type) { this->type = type; }
};

ostream& operator<<(ostream& out, Vertex& v) {
	out << "(" << v.x << ", " << v.y << "), id: " << v.id << ", spec: " << v.spec << ", typ: " << v.type << endl;
	return out;
}

vector<Vertex> loadFileData(string file_name) {
	vector<Vertex> data;
	ifstream input_file;
	input_file.open(file_name);
	int t_x, t_y;
	unsigned t_id, t_spec;
	while (input_file >> t_x >> t_y >> t_id >> t_spec) {
		Vertex temp(t_x, t_y, t_id, t_spec);
		data.emplace_back(temp);
	}
}

template<typename T>
void printVector(vector<T> vec) {
	for (auto it = begin(vec), it != end(vec); it++) {
		cout << *it;
	}
}

void initTypesDict(map<int, char> typemap) {
	typemap.insert({ 1, 'b' });
	typemap.insert({ 2, 'k' });
	typemap.insert({ 3, 'p' });
	typemap.insert({ 4, 's' });
}

int main() {
	map<int, char> typemap;
	
	return 0;
}
