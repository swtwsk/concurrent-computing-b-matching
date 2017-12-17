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

    struct PointerComparator {
        bool operator() (Edge*a, Edge*b) {
            return a->weight == b->weight ? (a->v < b->v) : (a->weight < b->weight);
        }
    };
};
bool operator<(const Edge& a, const Edge& b) { // operator for MIN priority_queue
    if (a.weight == b.weight) {
        return a.v < b.v;
    }
    return a.weight < b.weight;
}
std::map<int, std::map<int, Edge>> graph_edges;

struct Verticle {
    int id;
    unsigned int max_weight;
    unsigned int bvalue;
    std::priority_queue<Edge *, std::vector<Edge *>, Edge::PointerComparator> S;
    std::map<int, unsigned int> T;

    explicit Verticle(int id, unsigned int b_val) : id(id), bvalue(b_val) {
        max_weight = 0;
        for (auto it = graph_edges[id].begin(); it != graph_edges[id].end(); ++it) {
            max_weight = it->second.weight > max_weight ? it->second.weight : max_weight;
        }
    }

    bool hasLast() {
        return S.size() == bvalue;
    }

    struct PointerComparator {
        bool operator() (Verticle* a, Verticle* b) {
            return a->max_weight < b->max_weight;
        }
    };
};
std::map<int, Verticle> graph_vert;
std::priority_queue<Verticle *, std::vector<Verticle *>, Verticle::PointerComparator> Q;
std::queue<Verticle *> R;

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

                graph_edges[id_fvert].emplace(id_svert, Edge(id_svert, weight));
                graph_edges[id_svert].emplace(id_fvert, Edge(id_fvert, weight));
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

std::map<int, Edge>::iterator findMaxEdge(int id) {
    std::map<int, Edge>::iterator eligible = graph_edges[id].begin();
    bool found = false;

    for (auto it = graph_edges[id].begin(); it != graph_edges[id].end(); ++it) {
        if (!it->second.used) {
            auto v = graph_vert.find(it->second.v);
            if (v != graph_vert.end() && v->second.hasLast() && it->second < *(v->second.S.top())) {
                continue;
            }
            if (eligible->second.weight <= it->second.weight) {
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

        debug << vert->id << " " << vert->bvalue << std::endl;

        while (vert->S.size() < vert->bvalue) {
            auto x_it = findMaxEdge(vert->id);
            if (x_it == graph_edges[vert->id].end()) {
                break;
            }
            else {
                auto& x = x_it->second;
                debug << " size=" << vert->S.size() << ", x=" << x.v;
                vert->S.push(&x_it->second);
                vert->T.insert(std::make_pair(x.v, x.weight));
                x.used = true;
                auto& to_propose = graph_vert.find(x.v)->second;
                auto& suitors = to_propose.S;
                if (to_propose.hasLast()) {
                    auto& y = graph_vert.find(suitors.top()->v)->second;
                    debug << ", y=" << y.id;
                    y.T.erase(to_propose.id);
                    graph_edges.find(y.id)->second.find(x.v)->second.used = false;
                    R.push(&y);
                    suitors.pop();
                }
                suitors.push(&(graph_edges.find(x.v)->second.find(vert->id)->second));
                debug << std::endl;
            }
        }

        if (Q.empty()) {
            while (!R.empty()) {
                Q.push(R.front());
                R.pop();
            }
        }
    }

    debug << std::endl;
    unsigned int res = 0;
    for (auto it = graph_vert.begin(); it != graph_vert.end(); ++it) {
        std::cout << it->first << ":\n";
        for (auto m_it = it->second.T.begin(); m_it != it->second.T.end(); ++m_it) {
            std::cout << " " << m_it->first << " " << m_it->second << "\n";
            res += m_it->second;
        }
        /*while (!it->second.S.empty()) {
            unsigned int top_weight = it->second.S.top()->weight;
            std::cout << " " << it->second.S.top()->v << " " << top_weight << "\n";
            res += top_weight;
            it->second.S.pop();
        }*/
    }

    std::cout << "RES: " << res << std::endl;

    graph_vert.clear();
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

        sequentialAlgorithm(b_method);
        debug << std::endl;

        // TODO: implement b-adorators here

        // fake result
        //std::cout << 42 << std::endl;
    }
}