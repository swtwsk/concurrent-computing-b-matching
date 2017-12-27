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

bool compareVerticles(const VerticleType a, const VerticleType b) {
	return max_weight[graph[a]] > max_weight[graph[b]];
}

bool compareEdgesForSort(const Edge& a, const Edge& b) {
	return a.weight > b.weight;
}

inline bool compareEdges(const Edge& u_v, const Edge& v_last, VerticleType u) {
	return u_v.weight == v_last.weight ? u < v_last.to : u_v.weight < v_last.weight;
}

inline VerticleType findX(VerticleType u) {
	Edge eligible = Edge(-1, 0);
	VerticleType x = -1;
	auto u_id = graph[u];

	for (auto i = 0; i < edges[u_id].size(); ++i) {
		auto& edge = edges[u_id][i];
		if (b_value[graph[edge.to]] == 0) {
			continue;
		}

		if (T[u_id].find(edge.to) == T[u_id].end()) {
			if (hasLast(edge.to) && compareEdges(edge, S[graph[edge.to]].top(), u)) {
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
	for (auto v : V) {
		b_value[graph[v]] = bvalue(b_method, v);
		if (b_value[graph[v]] != 0) {
			Q.push(v);
		}
	}

	while (!Q.empty()) {
		auto u = Q.front();
		auto u_ind = graph[u];
		Q.pop();

		//debug << u << " - " << u_ind << std::endl;

		while (T[u_ind].size() < b_value[u_ind]) {
			auto x_in_u_ind = findX(u);
			if (x_in_u_ind < 0) {
				break;
			}
			else { // makeSuitor(u, x)
				auto x = edges[u_ind][x_in_u_ind].to;
				auto x_ind = graph[x];
				//debug << ' ' << x << " - " << x_ind << ", S.size=" << S[x_ind].size() << '\n';
				VerticleType y = -1;
				if (hasLast(x)) {
					y = S[x_ind].top().to;
					S[x_ind].pop();
				}

				S[x_ind].push(Edge(u, edges[u_ind][x_in_u_ind].weight));
				T[u_ind].insert(x);

				if (y >= 0) {
					T[graph[y]].erase(x);
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
		auto& Sv = S[v.second];
		while (!Sv.empty()) {
			res += Sv.top().weight;
			Sv.pop();
		}
		T[v.second].clear();
	}

	std::cout << res / 2 << std::endl;
	return res / 2;
}

inline void createVerticle(VerticleType id) {
	if (graph.find(id) == graph.end()) {
		auto ind = edges.size();
		graph.insert(std::make_pair(id, ind));

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
			//v.second.findMaxWeight();
			std::sort(edges[v.second].begin(), edges[v.second].end(), compareEdgesForSort);
			max_weight[v.second] = edges[v.second].begin()->weight;
			V.push_back(v.first);
		}

		std::sort(V.begin(), V.end(), compareVerticles);

		file.close();
	}
}

void printGraph() {
	for (auto& v : graph) {
		debug << v.first << " - " << v.second << ":";
		for (auto& e : edges[v.second]) {
			debug << " (" << e.to << ", " << e.weight << ");";
		}
		debug << '\n';
	}
	debug << std::endl;
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
	//printGraph();

	for (int b_method = 0; b_method < b_limit + 1; b_method++) {
		std::cerr << "B = " << b_method << std::endl;

		sequentialAlgorithm(b_method);
	}
}