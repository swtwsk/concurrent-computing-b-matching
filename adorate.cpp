#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <limits>
#include <algorithm>

#include <thread>
#include <mutex>

#include "blimit.hpp"

#define debug std::cout

using VerticleType = int;
using WeightType = unsigned int;

struct Edge {
    VerticleType to;
    WeightType weight;

    explicit Edge(VerticleType to, WeightType weight) : to(to), weight(weight) {};

    bool operator<(const Edge& other) {
        return weight == other.weight ? to < other.to : weight < other.weight;
    }

    struct Comparator {
        constexpr bool operator() (const Edge& a, const Edge& b) const {
            return a.weight == b.weight ? a.to > b.to : a.weight > b.weight;
        }
    };
};

struct Verticle {
    std::vector<Edge> edges;
    std::set<VerticleType> T;
    std::priority_queue<Edge, std::vector<Edge>, Edge::Comparator> S;

    WeightType max_weight;
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

std::map<VerticleType, Verticle> graph; // graph representation
std::vector<VerticleType> V; // sorted verticles

bool compareVerticles (const VerticleType a, const VerticleType b) {
    return graph[a].max_weight > graph[b].max_weight;
}

bool compareEdgesForSort(const Edge& a, const Edge& b) {
    return a.weight > b.weight;
}

inline bool compareEdges(const Edge& u_v, const Edge& v_last, int u) {
    return u_v.weight == v_last.weight ? u < v_last.to : u_v.weight < v_last.weight;
}

int findX(VerticleType id) {
    Edge eligible = Edge(-1, 0);
    VerticleType x = -1;

    for (auto i = 0; i < graph[id].edges.size(); ++i) {
        auto& edge = graph[id].edges[i];

        if (graph[id].T.find(edge.to) == graph[id].T.end()) {
            if (graph[edge.to].hasLast() && compareEdges(edge, graph[edge.to].S.top(), id)) {//edge < graph[edge.to].S.top()) {//
                continue;
            }
            if (eligible < edge) {
                eligible = edge;
                x = i;
            }
        }
    }

    return x;
}

unsigned int sequentialAlgorithm(unsigned int b_method) {
    std::queue<int> Q;
    std::queue<int> R;

    for (auto v : V) {
        Q.push(v);
        graph[v].b_value = bvalue(b_method, v);
    }

    while (!Q.empty()) {
        auto u = Q.front();
        Q.pop();

        while (graph[u].T.size() < graph[u].b_value) {
            auto x_id = findX(u);
            if (x_id < 0) {
                break;
            }
            else { // makeSuitor(u, x)
                auto x = graph[u].edges[x_id].to;
                int y = -1;
                if (graph[x].hasLast()) {
                    y = graph[x].S.top().to;
                    graph[x].S.pop();
                }

                graph[x].S.push(Edge(u, graph[u].edges[x_id].weight));
                graph[u].T.insert(x);

                if (y >= 0) {
                    graph[y].T.erase(x);
                    R.push(y);
                }
            }
        }

        if (Q.empty()) {
            while (!R.empty()) {
                Q.push(R.front());
                R.pop();
            }
        }
    }

    unsigned int res = 0;

    for (auto& v : graph) {
        while (!v.second.S.empty()) {
            res += v.second.S.top().weight;
            v.second.S.pop();
        }
        v.second.T.clear();
    }

    debug << res / 2 << std::endl;

    return res / 2;

    //debug << "Done" << std::endl;
}

inline void createVerticle(VerticleType id) {
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
                VerticleType id_fvert;
                VerticleType id_svert;
                WeightType weight;

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
            std::sort(v.second.edges.begin(), v.second.edges.end(), compareEdgesForSort);
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