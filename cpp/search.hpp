#pragma once

#include "pool.hpp"
#include "util.hpp"
#include <random>

struct SearchState {
	Position pos;
	double 
};

struct Searcher {
	int max_pty, n, m;
	vec<vec<vec<uint64_t>>> pt_pos_hash;
	Pool pool;
	
	Searcher(int max_pty_, int n_, int m_, int nt_) : max_pty(max_pty_), n(n_), m(m_), pool(nt_) {
		std::mt19937_64 rng(123);
		pt_pos_hash.resize(max_pty);
		for (int pt=0; pt<max_pty; pt++) {
			pt_pos_hash[pt].assign(n, vec<uint64_t>(m));
			for (int i=0; i<n; i++) for (int j=0; j<m; j++) pt_pos_hash[pt][i][j]=rng();
		}
	}


};