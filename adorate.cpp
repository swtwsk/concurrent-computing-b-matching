#include "blimit.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>

#define MAXN 1000000 // tu jest jazda, bo 10^9 to juz za duza tablica

#define debug std::cout

struct Edge {
    unsigned int v; // Verticle the edge is pointing to
    unsigned int weight; // Weight of the Edge

    explicit Edge(unsigned int v, unsigned int weight) : v(v), weight(weight) {};
};

std::vector<Edge> V[MAXN];

unsigned int n = 0; // for debug purposes

void parseFile(std::string &input_filename) {
    std::ifstream file;
    file.open(input_filename, std::ifstream::in);

    if (file.is_open()) {
        debug << input_filename << " has been opened\n";
        char c;
        while (file.get(c)) {

            if (c != '#') {
                file.unget();
                unsigned int id_fvert;
                unsigned int id_svert;
                unsigned int weight;

                file >> id_fvert >> id_svert >> weight;
                V[id_fvert].push_back(Edge(id_svert, weight));
                V[id_svert].push_back(Edge(id_fvert, weight));

                unsigned int max_id = id_fvert >= id_svert ? id_fvert : id_svert;
                n = n < max_id ? max_id : n;
            }
            else {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        file.close();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " thread-count inputfile b-limit" << std::endl;
        return 1;
    }

    std::cout << "NO ELO" << std::endl;

    int thread_count = std::stoi(argv[1]);
    int b_limit = std::stoi(argv[3]);
    std::string input_filename{ argv[2] };
    parseFile(input_filename);

    for (auto i = 0; i <= n; ++i) {
        std::cout << i << ":\n";
        for (auto it = V[i].begin(); it != V[i].end(); ++it) {
            std::cout << " (" << (*it).v << ", " << (*it).weight << ")\n";
        }
    }

    for (int b_method = 0; b_method < b_limit + 1; b_method++) {
        // this is just to show the blimit with which the program is linked
        std::cerr << "bvalue node 44: " << bvalue(b_method, 44) << std::endl;

        // TODO: implement b-adorators here

        // fake result
        std::cout << 42 << std::endl;
    }
}