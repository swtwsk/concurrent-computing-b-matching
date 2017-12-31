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

using VerticleType = int;
using WeightType = unsigned int;

struct Edge {
    VerticleType to;
    WeightType weight;

    explicit Edge(VerticleType to, WeightType weight) : to(to), weight(weight) {};

    constexpr bool operator<(const Edge& other) {
        return weight == other.weight ? to < other.to : weight < other.weight;
    }

    struct Comparator {
        constexpr bool operator() (const Edge& a, const Edge& b) const {
            return a.weight == b.weight ? a.to > b.to : a.weight > b.weight;
        }
    };
};

/* FOR VERTEXES REPRESENTATION: */
using EdgeCont = std::vector<Edge>;
using TSet = std::set<VerticleType>;
using SQue = std::priority_queue<Edge, std::vector<Edge>, Edge::Comparator>;

std::map<VerticleType, VerticleType> graph; // map from vertex id to vector index
/* VERTEXES REPRESENTATION: */
std::vector<EdgeCont> edges;
std::vector<TSet> T;
std::vector<SQue> S;
std::vector<WeightType> max_weight;
std::vector<unsigned int> b_value;

std::vector<VerticleType> V; // sorted vertexes

inline bool hasLast(VerticleType v) {
    auto v_id = graph[v];
    return S[v_id].size() == b_value[v_id];
}

/* QUEUES */
std::queue<VerticleType> Q;
std::queue<VerticleType> R;

/* MUTEXES */
struct mutex_wrapper : std::mutex
{
    mutex_wrapper() = default;
    mutex_wrapper(mutex_wrapper const&) noexcept : std::mutex() {}
    bool operator==(mutex_wrapper const&other) noexcept { return this == &other; }
};

std::map<VerticleType, mutex_wrapper> SMutexes;
std::map<VerticleType, mutex_wrapper> TMutexes;
std::mutex Qmutex;

bool compareVerticles(const VerticleType a, const VerticleType b) {
    return max_weight[graph[a]] > max_weight[graph[b]];
}

bool compareEdgesForSort(const Edge& a, const Edge& b) {
    return b < a;
}

inline bool compareEdges(const Edge& u_v, const Edge& v_last, VerticleType u) {
    return u_v.weight == v_last.weight ? u < v_last.to : u_v.weight < v_last.weight;
}

inline VerticleType findX(VerticleType u) {
    auto u_id = graph[u];

    for (EdgeCont::size_type i = 0; i < edges[u_id].size(); ++i) {
        auto& edge = edges[u_id][i];
        if (b_value[graph[edge.to]] == 0) {
            continue;
        }

        if (T[u_id].find(edge.to) == T[u_id].end()) {
            {
                std::lock_guard<std::mutex> s_lock(SMutexes[edge.to]);
                if (hasLast(edge.to) && compareEdges(edge, S[graph[edge.to]].top(), u)) {
                    continue;
                }
                else {
                    return i;
                }
            }
        }
    }

    return -1;
}

bool stillEligible(VerticleType id, VerticleType x_ind) {
    auto v_ind = graph[id];
    auto& x = edges[v_ind][x_ind];

    if (T[v_ind].find(x.to) != T[v_ind].end()) {
        return false;
    }
    if (hasLast(x.to) && compareEdges(x, S[graph[x.to]].top(), id)) {
        return false;
    }

    return true;
}

void concurrentAlgorithm() {
    VerticleType u;
    while (true) {
        {
            std::lock_guard<std::mutex> q_lock(Qmutex);

            if (Q.empty()) {
                if (R.empty()) {
                    return;
                }
                else {
                    while (!R.empty()) {
                        Q.push(R.front());
                        R.pop();
                    }
                }
            }
            else {
                u = Q.front();
                Q.pop();
            }
        } // RAII

        auto u_ind = graph[u];
        std::lock_guard<std::mutex> u_lock(TMutexes[u]);
        while (T[u_ind].size() < b_value[u_ind]) {
            auto x_in_u_ind = findX(u);
            if (x_in_u_ind < 0) {
                break;
            }
            else { // makeSuitor(u, x)
                VerticleType y = -1;

                auto x = edges[u_ind][x_in_u_ind].to;
                auto x_ind = graph[x];

                { //RAII - lock x
                    std::lock_guard<std::mutex> x_lock(SMutexes[x]);

                    if (!stillEligible(u, x_in_u_ind)) {
                        continue;
                    }

                    if (hasLast(x)) {
                        y = S[x_ind].top().to;
                        S[x_ind].pop();
                    }

                    S[x_ind].push(Edge(u, edges[u_ind][x_in_u_ind].weight));
                    T[u_ind].insert(x);
                }

                if (y >= 0) {
                    {
                        std::lock_guard<std::mutex> y_lock(TMutexes[y]);
                        T[graph[y]].erase(x);
                    }

                    {
                        std::lock_guard<std::mutex> q_lock(Qmutex);
                        R.push(y);
                    }
                }
            }
        }
    }
}

void countBValue(int nr, unsigned int b_method,
    std::vector<std::vector<VerticleType>>& bValueToCount) {

    for (auto v : bValueToCount[nr]) {
        b_value[graph[v]] = bvalue(b_method, v);
    }
}

void concurrentAdministrator(unsigned int b_method, int max_thread_count,
    std::vector<std::vector<VerticleType>>& bValueToCount) {

    std::vector<std::thread> threads;
    // count b-values
    if (max_thread_count > 0) {
        for (auto j = 0; j < max_thread_count; ++j) {
            threads.push_back(std::thread{ [j, b_method, &bValueToCount] {
                countBValue(j, b_method, bValueToCount); } });
        }
        std::for_each(threads.begin(), threads.end(), [](std::thread &t)
        {
            t.join();
        });
    }
    else {
        for (auto v : V) {
            b_value[graph[v]] = bvalue(b_method, v);
        }
    }

    for (auto v : V) {
        if (b_value[graph[v]] != 0) {
            Q.push(v);
        }
    }

    // find adorators
    for (int i = 0; i < max_thread_count; ++i) {
        threads[i] = std::thread{ concurrentAlgorithm };
    }

    concurrentAlgorithm();

    std::for_each(threads.begin(), threads.end(), [](std::thread &t)
    {
        t.join();
    });

    // count weight of b-matching
    unsigned int res = 0;

    for (auto& v : graph) {
        auto& Sv = S[v.second];
        while (!Sv.empty()) {
            res += Sv.top().weight;
            Sv.pop();
        }
        T[v.second].clear();
    }

    std::cout << res / 2 << std::endl;
}

inline void createVerticle(VerticleType id) {
    if (graph.find(id) == graph.end()) {
        graph.emplace(id, edges.size());
        SMutexes.emplace(id, mutex_wrapper());
        TMutexes.emplace(id, mutex_wrapper());

        edges.push_back(EdgeCont());
        T.push_back(TSet());
        S.push_back(SQue());
        max_weight.push_back(0);
        b_value.push_back(0);
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

                edges[graph[id_fvert]].push_back(Edge(id_svert, weight));
                edges[graph[id_svert]].push_back(Edge(id_fvert, weight));
            }
            else {
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }

        for (auto& v : graph) {
            std::sort(edges[v.second].begin(), edges[v.second].end(), compareEdgesForSort);
            max_weight[v.second] = edges[v.second].begin()->weight;
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

    auto max_thread_count = (unsigned int) (thread_count - 1) > V.size() ? V.size() : (thread_count - 1);

    std::vector<std::vector<VerticleType>> bValueToCount;
    if (max_thread_count > 0) {
        bValueToCount.resize(max_thread_count);

        for (std::vector<VerticleType>::size_type j = 0; j < V.size(); j += max_thread_count) {
            auto max_i = std::min(max_thread_count, V.size() - j);
            for (unsigned int i = 0; i < max_i; ++i) {
                bValueToCount[i].push_back(V[j + i]);
            }
        }
    }

    for (int b_method = 0; b_method < b_limit + 1; b_method++) {
        concurrentAdministrator(b_method, max_thread_count, bValueToCount);
    }
}
