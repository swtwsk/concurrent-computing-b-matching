#include "blimit.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <limits>

#define debug std::cout

struct Edge {
    int v; // verticle the edge is pointing to
    unsigned int weight; // weight of the Edge
    bool used;

    explicit Edge(int v, unsigned int weight) : v(v), weight(weight), used(false) {};
};
bool operator<(const Edge& a, const Edge& b) { // operator for MIN priority_queue
    if (a.weight == b.weight) {
        return a.v > b.v;
    }
    return a.weight > b.weight;
}
std::map<int, std::vector<Edge>> graph_edges;

struct Verticle {
    int id;
    unsigned int max_weight;
    unsigned int bvalue;
    std::priority_queue<Edge> S;

    explicit Verticle(int id, unsigned int b_val) : id(id), bvalue(b_val) {
        max_weight = 0;
        for (auto it = graph_edges[id].begin(); it != graph_edges[id].end(); ++it) {
            max_weight = (*it).weight > max_weight ? (*it).weight : max_weight;
        }
    }

    bool hasLast() {
        return S.size() == bvalue;
    }
};
bool operator<(const Verticle& a, const Verticle& b) {
    return a.max_weight < b.max_weight;
}
std::map<int, Verticle> graph_vert;
std::priority_queue<Verticle *> Q;

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

                graph_edges[id_fvert].push_back(Edge(id_svert, weight));
                graph_edges[id_svert].push_back(Edge(id_fvert, weight));
            }
            else {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        file.close();
    }
}

inline void prepareQueue(int b_method) {
    for (auto m_it = graph_edges.begin(); m_it != graph_edges.end(); ++m_it) {
        auto id = (*m_it).first;
        graph_vert.emplace(id, Verticle(id, bvalue(b_method, id)));
        Q.push(&graph_vert.find(id)->second);
    }
}

std::vector<Edge>::iterator findMaxVerticle(int id) {
    std::vector<Edge>::iterator eligible = graph_edges[id].begin(); // weight, v
    bool found = false;

    for (auto it = graph_edges[id].begin(); it != graph_edges[id].end(); ++it) {
        if (!(*it).used) {
            if ((*eligible).weight <= (*it).weight) {
                eligible = it;
                found = true;
            }
        }
    }

    return found ? eligible : graph_edges[id].end();
}

void sequentialAlgorithm(int b_method) {
    prepareQueue(b_method);

    while (!Q.empty()) {
        auto vert = Q.top();
        Q.pop();

        //debug << vert.id << " " << vert.bvalue << std::endl;

        while (vert->S.size() < vert->bvalue) {
            auto x = findMaxVerticle(vert->id);
            if (x == graph_edges[vert->id].end()) {
                break;
            }
            else {
                vert->S.push((*x));
                (*x).used = true;
                auto& to_propose = graph_vert[(*x).v];
                auto& suitors = to_propose.S;
                if (to_propose.hasLast()) {
                    auto y = suitors.top();
                    suitors.pop();
                    suitors.push()
                }
            }
        }
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
        std::cerr << "bvalue node 44: " << bvalue(b_method, 44) << std::endl;

        sequentialAlgorithm(b_method);

        // TODO: implement b-adorators here

        // fake result
        std::cout << 42 << std::endl;
    }
}