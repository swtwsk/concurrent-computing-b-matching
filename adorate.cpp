#include "blimit.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <limits>

#define debug std::cout

struct Edge {
    int to;
    unsigned int weight;

    explicit Edge(int to, unsigned int weight) : to(to), weight(weight) {};

    bool operator<(const Edge& other) {
        return weight == other.weight ? to < other.to : weight < other.weight;
    }

    struct Comparator {
        constexpr bool operator() (const Edge& a, const Edge& b) const {
            return a.weight == b.weight ? a.to < b.to : a.weight < b.weight;
        }
    };
};

struct Verticle {
    std::vector<Edge> edges;
    std::set<int> T;
    std::priority_queue<Edge, std::vector<Edge>, Edge::Comparator> S;

    unsigned int max_weight;
    unsigned int b_value;

    void findMaxWeight() {
        max_weight = 0;
        for (auto& edge : edges) {
            max_weight = edge.weight > max_weight ? edge.weight : max_weight;
        }
    }

    bool hasLast() {
        return S.size() == b_value;
    }
};

std::map<int, Verticle> graph; // graph representation
std::vector<int> V; // sorted verticles

bool compareVerticles (const int a, const int b) {
    return graph[a].max_weight > graph[b].max_weight;
}

int findX(int id) {
    Edge eligible = Edge(-1, 0);
    int x = -1;

    //debug << " findX:\n";
    for (auto i = 0; i < graph[id].edges.size(); ++i) {
        auto& edge = graph[id].edges[i];

        //debug << edge.to;
        if (graph[id].T.find(edge.to) == graph[id].T.end()) {
            //debug << "+ eli:" << eligible.weight << ", edg:" << edge.weight << "\n";
            if (graph[edge.to].hasLast() && edge < graph[edge.to].S.top()) {
                continue;
            }
            if (eligible.weight < edge.weight) {
                eligible = edge;
                x = i;
            }
        }
        //debug << " ";
    }
    //debug << x << std::endl;

    return x;
}

void sequentialAlgorithm(unsigned int b_method) {
    std::queue<int> Q;
    std::queue<int> R;

    for (auto v : V) {
        Q.push(v);
        graph[v].b_value = bvalue(b_method, v);
    }

    while (!Q.empty()) {
        auto u = Q.front();
        Q.pop();

        //debug << u << " " << graph[u].b_value << std::endl;

        auto& u_vert = graph[u];

        while (u_vert.T.size() < u_vert.b_value) {
            auto x_id = findX(u);
            if (x_id < 0) {
                break;
            }
            else {
                auto x = u_vert.edges[x_id].to;
                if (graph[x].hasLast()) {
                    auto y = graph[x].S.top();
                    graph[x].S.pop();
                    u_vert.T.erase(y.to);
                    R.push(y.to);
                }
                graph[x].S.push(Edge(u, u_vert.edges[x_id].weight));
                u_vert.T.insert(x);
            }
        }

        if (Q.empty()) {
            while (!R.empty()) {
                Q.push(R.front());
                R.pop();
            }
        }
    }

    for (auto& v : graph) {
        debug << v.first << ": ";
        for (auto t : v.second.T) {
            debug << t << " ";
        }
        debug << std::endl;
        v.second.T.clear();
    }

    //debug << "Done" << std::endl;
}

inline void createVerticle(int id) {
    if (graph.find(id) == graph.end()) {
        graph.insert(std::make_pair(id, Verticle()));
    }
}

void parseFile(std::string &input_filename) {
    std::ifstream file;
    file.open(input_filename, std::ifstream::in);

    if (file.is_open()) {
        char c;

        while (file.get(c)) {
            if (c != '#') {
                file.unget();
                int id_fvert;
                int id_svert;
                unsigned int weight;

                file >> id_fvert >> id_svert >> weight;
                file.ignore();

                createVerticle(id_fvert);
                createVerticle(id_svert);

                graph[id_fvert].edges.push_back(Edge(id_svert, weight));
                graph[id_svert].edges.push_back(Edge(id_fvert, weight));
            }
            else {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        for (auto& v : graph) {
            v.second.findMaxWeight();
            V.push_back(v.first);
        }

        std::sort(V.begin(), V.end(), compareVerticles);

        file.close();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " thread-count inputfile b-limit" << std::endl;
        return 1;
    }

    int thread_count = std::stoi(argv[1]);
    int b_limit = std::stoi(argv[3]);
    std::string input_filename{ argv[2] };
    parseFile(input_filename);

    for (int b_method = 0; b_method < b_limit + 1; b_method++) {
        // this is just to show the blimit with which the program is linked
        //std::cerr << "bvalue node 44: " << bvalue(b_method, 44) << std::endl;

        std::cerr << "B = " << b_method << std::endl;
        sequentialAlgorithm(b_method);
        //debug << std::endl;

        // TODO: implement b-adorators here

        // fake result
        //std::cout << 42 << std::endl;
    }
}