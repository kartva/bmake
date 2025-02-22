#pragma once

#include "lua_interface.hpp"
#include "nn.hpp"
#include "pool.hpp"
#include "util.hpp"
#include <mutex>
#include <random>

vec<char> piece_names = {
	'P', 'N', 'B', 'R', 'Q', 'K'
};

vec<int> piece_weights = {
	100, 280, 320, 479, 929, 60000
};

vec<vec<int>> pst = {
	{   0,   0,   0,   0,   0,   0,   0,   0,
		78,  83,  86,  73, 102,  82,  85,  90,
		 7,  29,  21,  44,  40,  31,  44,   7,
		 -17,  16,  -2,  15,  14,   0,  15, -13,
		 -26,   3,  10,   9,   6,   1,   0, -23,
		 -22,   9,   5, -11, -10,  -2,   3, -19,
		 -31,   8,  -7, -37, -36, -14,   3, -31,
		 0,   0,   0,   0,   0,   0,   0,   0},
	{ -66, -53, -75, -75, -10, -55, -58, -70,
		-3,  -6, 100, -36,   4,  62,  -4, -14,
		10,  67,   1,  74,  73,  27,  62,  -2,
		24,  24,  45,  37,  33,  41,  25,  17,
		-1,   5,  31,  21,  22,  35,   2,   0,
		 -18,  10,  13,  22,  18,  15,  11, -14,
		 -23, -15,   2,   0,   2,   0, -23, -20,
		 -74, -23, -26, -24, -19, -35, -22, -69},
	{ -59, -78, -82, -76, -23,-107, -37, -50,
		 -11,  20,  35, -42, -39,  31,   2, -22,
		-9,  39, -32,  41,  52, -10,  28, -14,
		25,  17,  20,  34,  26,  25,  15,  10,
		13,  10,  17,  23,  17,  16,   0,   7,
		14,  25,  24,  15,   8,  25,  20,  15,
		19,  20,  11,   6,   7,   6,  20,  16,
		-7,   2, -15, -12, -14, -15, -10, -10},
	{  35,  29,  33,   4,  37,  33,  56,  50,
		55,  29,  56,  67,  55,  62,  34,  60,
		19,  35,  28,  33,  45,  27,  25,  15,
		 0,   5,  16,  13,  18,  -4,  -9,  -6,
		 -28, -35, -16, -21, -13, -29, -46, -30,
		 -42, -28, -42, -25, -25, -35, -26, -46,
		 -53, -38, -31, -26, -29, -43, -44, -53,
		 -30, -24, -18,   5,  -2, -18, -31, -32},
	{   6,   1,  -8,-104,  69,  24,  88,  26,
		14,  32,  60, -10,  20,  76,  57,  24,
		-2,  43,  32,  60,  72,  63,  43,   2,
		 1, -16,  22,  17,  25,  20, -13,  -6,
		 -14, -15,  -2,  -5,  -1, -10, -20, -22,
		 -30,  -6, -13, -11, -16, -11, -16, -27,
		 -36, -18,   0, -19, -15, -15, -21, -38,
		 -39, -30, -31, -13, -31, -36, -34, -42},
	{   4,  54,  47, -99, -99,  60,  83, -62,
		 -32,  10,  55,  56,  56,  55,  10,   3,
		 -62,  12, -57,  44, -67,  28,  37, -31,
		 -55,  50,  11,  -4, -19,  13,   0, -49,
		 -55, -43, -52, -28, -51, -47,  -8, -50,
		 -47, -42, -43, -79, -64, -32, -29, -32,
		-4,   3, -14, -50, -57, -18,  13,   4,
		17,  30,  -3, -14,   6,  -1,  40,  18}
};

constexpr int LOSING = -1e6;
constexpr int WINNING = 1e6;

constexpr int QS = 40;
constexpr int QS_A = 140;
constexpr int EVAL_ROUGHNESS = 15;

struct SearchState {
	Position pos;
	uint64_t hash;
	
	int score;
	int depth;
	bool visited;

	int parent;
	
};

struct SearchStateCache {
	// lo <= optimal score <= hi
	int lo, hi;
};

#ifdef BUILD_DEBUG
template<class V>
using SearchAlloc = std::allocator<std::pair<uint64_t, V>>;
#else
template<class V>
using SearchAlloc = mi_stl_allocator<std::pair<uint64_t, V>>;
#endif

struct Searcher {
	int max_pty, n, m, max_depth;
	vec<vec<vec<uint64_t>>> pt_pos_hash;
	vec<uint64_t> depth_hash;

	Pool pool;
	LuaInterface& interface;

	gtl::parallel_flat_hash_map<uint64_t, int, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<int>, 6, std::mutex> killer_move;

	gtl::parallel_flat_hash_map<uint64_t, SearchStateCache, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<SearchStateCache>, 6, std::mutex> cache;
	
	Searcher(int max_pty_, int n_, int m_, int max_depth_, int nt_, LuaInterface& interface_):
		max_pty(max_pty_), n(n_), m(m_), max_depth(max_depth_), pool(nt_), interface(interface_) {

		std::mt19937_64 rng(123);

		pt_pos_hash.resize(max_pty);
		for (int pt=0; pt<max_pty; pt++) {
			pt_pos_hash[pt].assign(n, vec<uint64_t>(m));
			for (int i=0; i<n; i++) for (int j=0; j<m; j++) pt_pos_hash[pt][i][j]=rng();
		}

		depth_hash.resize(max_depth+1);
		for (int i=0; i<=max_depth; i++) {
			depth_hash[i] = rng();
		}
	}

	void change(SearchState& state, Piece p) {
		auto& pos = state.pos;
		if (p.being_added) {
			pos.board.push_back(p);
			state.score+=pst[p.type][p.c.i + n*p.c.j];
		} else {
			for (int idx=0; idx<pos.board.size(); idx++) {
				auto& p2 = pos.board[idx];
				if (p2.type == p.type && p2.c.i==p.c.i && p2.c.j==p.c.j) {
					std::swap(pos.board[idx], pos.board[pos.board.size()-1]);
					break;
				}
			}

			state.score-=pst[p.type][p.c.i + n*p.c.j];
			pos.board.pop_back();
		}
	}

	void bound(int gamma) {
		std::mutex mut;
		vec<std::unique_ptr<SearchState>> stack;

		pool.launch_all([&](int ti) {
			std::unique_lock lock(mut);
			while (true) {
				lock.lock();
				if (stack.empty()) return;
				auto state = std::move(stack.back());
				stack.pop_back();
				lock.unlock();
				
				if 
			}

		}, pool.threads.size()+1);
	}
};