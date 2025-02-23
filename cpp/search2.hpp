#pragma once

#include "lua_interface.hpp"
#include "nn.hpp"
#include "pool.hpp"
#include "util.hpp"
#include <chrono>
#include <condition_variable>
#include <iterator>
#include <latch>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>

vec<char> piece_names = {
	'P', 'N', 'B', 'R', 'Q', 'K', 
	// 'p', 'n', 'b', 'r', 'q', 'k'
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

constexpr int LOSING = -1e5;
constexpr int WINNING = 1e5;

constexpr int QS = 40;
constexpr int QS_A = 140;
constexpr int EVAL_ROUGHNESS = 15;

struct SearchState {
	Position pos;
	uint64_t hash;
	int score;
	int depth;
	int buf_i;
};

struct SearchStateCache {
	// lo <= optimal score <= hi
	int lo, hi;
	SearchStateCache(): lo(LOSING), hi(WINNING) {}
};

#ifdef BUILD_DEBUG
template<class V>
using SearchAlloc = std::allocator<std::pair<uint64_t, V>>;
#else
template<class V>
using SearchAlloc = mi_stl_allocator<std::pair<uint64_t, V>>;
#endif

struct Searcher {
	struct Bufs {
		vec<Move> t1;
		vec<int> t2;
		vec<SearchState> t3;
	};

	int max_pty, n, m, max_depth;
	vec<vec<uint64_t>> pt_pos_hash;
	vec<uint64_t> depth_hash;
	vec<LuaInterface> interfaces;
	vec<vec<Bufs>> tmp;

	Pool pool;
	std::string const& lua_path;
	uint64_t player_hash;

	gtl::parallel_flat_hash_map<uint64_t, int, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<int>, 6, std::mutex> killer_move;

	gtl::parallel_flat_hash_map<uint64_t, SearchStateCache, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<SearchStateCache>, 6, std::mutex> cache;
	
	Searcher(int max_pty_, int n_, int m_, int max_depth_,
		int nt_, std::string const& lua_path_):

		max_pty(max_pty_), n(n_), m(m_), max_depth(max_depth_),
		tmp(nt_+1), pool(nt_), lua_path(lua_path_) {

		std::mt19937_64 rng(123);
		player_hash = rng();

		pt_pos_hash.resize(max_pty+1);
		for (int pt=0; pt<=max_pty; pt++) {
			pt_pos_hash[pt].resize(n*m);
			for (int i=0; i<n; i++) for (int j=0; j<m; j++) pt_pos_hash[pt][i*m + j]=rng();
		}

		depth_hash.resize(max_depth+1);
		for (int i=0; i<=max_depth; i++) {
			depth_hash[i] = rng();
		}

		for (int i=0; i<=nt_; i++) {
			interfaces.emplace_back(lua_path);
		}
	}

	uint64_t hash(Position const& pos) {
		uint64_t o=0;
		if (pos.next_player) o^=player_hash;
		for (int i=0; i<n*m; i++) {
			o^=pt_pos_hash[pos.board[i]][i];
		}

		return o;
	}

	void change(SearchState& state, Move& move) {
		state.hash=state.pos.next_player ? player_hash : 0;
		
		std::swap(move.board, state.pos.board);
		for (int i=0; i<n*m; i++) state.hash^=pt_pos_hash[state.pos.board[i]][i];

		state.pos.next_player^=1;
		state.score = score(state.pos); //FIXME: optimize when replace with nnue
	}

	int score(Position const& pos) {
		int o1=0,o2=0;
		for (int i=0; i<n; i++) for (int j=0; j<m; j++) {
			int x = i*m+j;
			if (pos.board[x]){
				if (pos.board[x] <= 6){
					//std::cout << pos.board[x] - 1 << ' ' << x << ' ' << pst[pos.board[x] - 1][x] << '\n';
					o1+=piece_weights[pos.board[x]-1] + pst[pos.board[x] - 1][x];
				}
				else {
					//std::cout << pos.board[x] - 6 - 1 << ' ' << x << ' ' << pst[pos.board[x] - 6 - 1][x] << '\n';
					o2+=piece_weights[pos.board[x]-6-1] + pst[pos.board[x] - 6 - 1][x];
				}
			}
		}
		return pos.next_player ? o2-o1 : o1-o2;
	}

	// ab with [gamma, gamma+1]
	// fails low: <gamma
	// fails high: >=gamma+1
	int bound(int lua_i, SearchState s, int gamma) {
		std::cerr << "bound " << s.depth << ' ' << s.score << ' ' << gamma << '\n';

		if (s.depth<0) s.depth=0;

		int best=LOSING, best_move_i=-1;

		uint64_t k = s.hash^depth_hash[s.depth];

		SearchStateCache cache_v = cache[k];
		if (cache_v.lo>=gamma) return cache_v.lo;
		else if (cache_v.hi<gamma) return cache_v.hi;

		if (best_move_i!=-1) return best;

		if (s.depth==0) best=std::max(best, s.score);

		int killer_i=-1;

		auto it = killer_move.find(s.hash);
		if (it==killer_move.end() && s.depth>=3) {
			s.depth-=3;
			bound(lua_i, s, gamma);
			s.depth+=3;

			assert(killer_move.contains(s.hash));
			killer_i = killer_move[s.hash];
		} else if (s.depth>=3) {
			killer_i = it->second;
		}

		int min_score = QS - QS_A*s.depth + s.score;
		
		if (s.buf_i>=tmp[lua_i].size()) tmp[lua_i].resize(s.buf_i+1);

		auto& b = tmp[lua_i];
		int bi = s.buf_i;
		b[bi].t1.clear(), b[bi].t3.clear();

		interfaces[lua_i].valid_moves(b[bi].t1, s.pos);

		for (Move& move: b[bi].t1) {
			SearchState& val = b[bi].t3.emplace_back(SearchState {
				.pos=Position {.next_player=!s.pos.next_player},
				.depth = s.depth-1,
				.buf_i=s.buf_i+1
			});

			std::copy(move.board, move.board+n*m, val.pos.board);
			val.hash = hash(val.pos);
			
			auto pty = interfaces[lua_i].get_pos_type(val.pos);

			if (pty==PosType::Win) val.score = WINNING;
			else if (pty==PosType::Loss) val.score = LOSING;
			else if (pty==PosType::Draw) val.score = 0;
			else val.score = score(val.pos);
		}

		if (b[bi].t2.size() < b[bi].t1.size()) b[bi].t2.resize(b[bi].t1.size());

		std::iota(b[bi].t2.begin(), b[bi].t2.begin()+b[bi].t1.size(), 0);
		std::sort(b[bi].t2.begin(), b[bi].t2.begin()+b[bi].t1.size(),
			[&t3=b[bi].t3](int a, int b){
				return t3[a].score < t3[b].score;
			}
		);

		auto ret = [&]() {
			cache.emplace(k, SearchStateCache());

			cache.modify_if(k, [best,gamma](std::pair<const uint64_t,SearchStateCache>& kv){
				auto& v = kv.second;
				if (best<gamma) v.hi = best;
				else v.lo = best;
			});

			if (best_move_i!=-1) {
				killer_move.insert_or_assign(s.hash, best_move_i);
			}
		};

		if (best>=gamma) {ret(); return best;}

		bool inc_killer = killer_i!=-1 && -b[bi].t3[killer_i].score >= min_score;
		assert(killer_i>=-1 && killer_i<int(b[bi].t1.size()));
		for (int j=inc_killer ? -1 : 0; j<b[bi].t1.size(); j++) {
			int i = j==-1 ? killer_i : b[bi].t2[j];
			assert(i>=0 && i<b[bi].t1.size());
			if (j>=0 && i==killer_i) continue;

			if (j>=0 && -b[bi].t3[i].score < min_score) break;

			if (j>=0 && s.depth<=1 && b[bi].t3[i].score >= 1-gamma && -b[bi].t3[i].score>best) {
				best=-b[bi].t3[i].score;
				best_move_i=i;

				break;
			}

			int nv = -bound(lua_i, b[bi].t3[i], 1-gamma);
			if (nv>best) best=nv, best_move_i=i;

			if (best>=gamma) {ret(); return best;}
		}

		ret();
		return best;
	}

	struct SearchOut {
		PosType pos_type;
		vec<Move> possible;
		int move_i=-1;
	};

	static constexpr int TIME_LIMIT = 10000;
	SearchOut search(Position const& current) {
		auto start = std::chrono::steady_clock::now();
		bool tle=false;

		SearchOut out;
		out.pos_type = interfaces[0].get_pos_type(current);
		if (out.pos_type!=PosType::Other) return out; // leaf

		interfaces[0].valid_moves(out.possible, current);

		SearchState init(
			current, hash(current),
			score(current), 0, 0
		);

		for (int depth=1; !tle && depth<=max_depth; depth++) {
			init.depth=depth;
			std::cerr<<"depth "<<depth<<", cache size "<<cache.size()<<std::endl;

			auto now = std::chrono::steady_clock::now();

			int lo=LOSING, hi=WINNING;
			while (!tle && hi-lo > EVAL_ROUGHNESS) {
				int mid = (hi+lo+1)/2;

				auto ret = bound(0, init, mid);

				if (ret >= mid) lo=mid;
				else hi=mid-1;

				tle|=std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count() > TIME_LIMIT;
			}

			auto it = killer_move.find(hash(current));
			if (it!=killer_move.end()) out.move_i=it->second;
		}

		return out;
	}
};