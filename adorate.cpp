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
    unsigned int v; // verticle the edge is pointing to
    unsigned int weight; // weight of the Edge

    explicit Edge(unsigned int v, unsigned int weight) : v(v), weight(weight) {};
};
bool operator<(const Edge& a, const Edge& b) { // operator for MIN priority_queue
    if (a.weight == b.weight) {
        return a.v > b.v;
    }
    return a.weight > b.weight;
}
std::map<unsigned int, std::vector<Edge>> graph;

struct Verticle {
    unsigned int id;
    unsigned int max_weight;
    unsigned int bvalue;
    std::priority_queue<Edge> S;

    explicit Verticle(unsigned int id, unsigned int b_val) : id(id), bvalue(b_val) {
        max_weight = 0;
        for (auto it = graph[id].begin(); it != graph[id].end(); ++it) {
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
std::priority_queue<Verticle> Q;

void parseFile(std::string &input_filename) {
    std::ifstream file;
    file.open(input_filename, std::ifstream::in);

    if (file.is_open()) {
        char c;

        while (file.get(c)) {
            if (c != '#') {
                file.unget();
                unsigned int id_fvert;
                unsigned int id_svert;
                unsigned int weight;

                file >> id_fvert >> id_svert >> weight;
                file.ignore();

                graph[id_fvert].push_back(Edge(id_svert, weight));
                graph[id_svert].push_back(Edge(id_fvert, weight));
            }
            else {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        file.close();
    }
}

void prepareQueue(int b_method) {
    for (auto m_it = graph.begin(); m_it != graph.end(); ++m_it) {
        Q.push(Verticle((*m_it).first, bvalue(b_method, (*m_it).first)));
    }
}

void sequentialAlgorithm(int b_method) {
    prepareQueue(b_method);

    while (!Q.empty()) {
        auto vert = Q.top();
        Q.pop();

        debug << vert.id << " " << vert.bvalue << std::endl;
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