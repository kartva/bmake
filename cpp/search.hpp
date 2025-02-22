#pragma once

#include "lua_interface.hpp"
#include "nn.hpp"
#include "pool.hpp"
#include "util.hpp"
#include <chrono>
#include <condition_variable>
#include <latch>
#include <memory>
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
constexpr int MIN_DEPTH = 20;

struct SearchState {
	PosType pos_ty;
	Position pos;
	uint64_t hash;
	int score;

	int depth;
	bool visited=false;

	std::condition_variable wait;
	std::mutex mut;
	int n_wait=0;

	SearchState* parent;
	int move_i;
	std::atomic_flag kill_children;
	int best=LOSING, best_move_i=-1;
	// bool can_null=false;

	SearchState(PosType pos_ty, Position pos, uint64_t hash, int score, int move_i, int depth, SearchState* parent):
		pos_ty(pos_ty), pos(pos), hash(hash), score(score), depth(depth), move_i(move_i), parent(parent) {}
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
	int max_pty, n, m, max_depth;
	vec<vec<uint64_t>> pt_pos_hash;
	vec<uint64_t> depth_hash;
	vec<LuaInterface> interfaces;

	Pool pool;
	std::string const& lua_path;
	Position current;
	uint64_t player_hash;

	gtl::parallel_flat_hash_map<uint64_t, int, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<int>, 6, std::mutex> killer_move;

	gtl::parallel_flat_hash_map<uint64_t, SearchStateCache, std::identity,
		std::equal_to<uint64_t>, SearchAlloc<SearchStateCache>, 6, std::mutex> cache;
	
	Searcher(int max_pty_, int n_, int m_, int max_depth_, int nt_, std::string const& lua_path_, Position current_):
		max_pty(max_pty_), n(n_), m(m_), max_depth(max_depth_),
		pool(nt_), lua_path(lua_path_), current(current_) {

		std::mt19937_64 rng(123);
		player_hash = rng();

		pt_pos_hash.resize(max_pty+1);
		for (int pt=0; pt<=max_pty; pt++) {
			pt_pos_hash[pt].resize(n*m);
			for (int i=0; i<n; i++) for (int j=0; j<m; j++) pt_pos_hash[pt][i*m + j]=rng();
		}

		depth_hash.resize(max_depth+MIN_DEPTH+1);
		for (int i=-MIN_DEPTH; i<=max_depth; i++) {
			depth_hash[i+MIN_DEPTH] = rng();
		}

		for (int i=0; i<=nt_; i++) {
			interfaces.emplace_back(lua_path);
		}
	}

	uint64_t hash(Position& pos) {
		uint64_t o=0;
		if (pos.next_player) o^=player_hash;
		for (int i=0; i<n*m; i++) {
			o^=pt_pos_hash[pos.board[i]][i];
		}

		return o;
	}

	void change(SearchState& state, Move& move) {
		auto& pos = state.pos;
		state.hash^=player_hash;
		
		for (int i=0; i<n*m; i++) {
			if (move.board[i] != pos.board[i]) {
				state.hash^=pt_pos_hash[pos.board[i]][i]^pt_pos_hash[move.board[i]][i];
				std::swap(move.board[i], pos.board[i]);
			}
		}

		state.pos.next_player^=1;
		state.score = score(state.pos); //FIXME: optimize when replace with nnue
	}

	int score(Position& pos) {
		int o=0;
		for (int i=0; i<n; i++) for (int j=0; j<m; j++) {
			int x = i*m+j;
			if (pos.board[x])
				o+=pst[pos.board[x]-1][(pos.next_player ? 8-i : i)*m + j];
		}
		return o;
	}
	int n_calls=0;

	// ab with [gamma, gamma+1]
	// fails low: <gamma
	// fails high: >=gamma+1
	std::optional<std::pair<Move, int>> bound(int gamma, int depth) {
		int init_player = current.next_player;

		std::mutex mut;
		vec<std::unique_ptr<SearchState>> stack;

		auto init = std::make_unique<SearchState>(
			interfaces[0].get_pos_type(current), current, hash(current),
			score(current), -1, depth, nullptr
		);

		if (init->pos_ty!=PosType::Other) return std::nullopt;

		vec<Move> init_moves;
		interfaces[0].valid_moves(init_moves, current);

		stack.push_back(std::move(init));
		int out_move_i=-1, out;

		pool.launch_all([&](int ti) {
			vec<Move> moves;
			vec<std::pair<int,std::unique_ptr<SearchState>>> tmp;
			auto& lua = interfaces[ti];

			std::unique_lock lock(mut, std::defer_lock_t());
			while (true) {
				lock.lock();
				if (stack.empty()) return;
				auto state = std::move(stack.back());
				auto& s = *state;
				std::unique_lock self_lock(s.mut);
				stack.pop_back();
				lock.unlock();

				int g = s.pos.next_player==init_player ? 1-gamma : gamma;

				auto update_parent = [&](int x) {
					if (s.best_move_i!=-1) {
						killer_move.insert_or_assign(s.hash, s.best_move_i);
					}

					uint64_t k = s.hash^depth_hash[MIN_DEPTH+s.depth];

					cache.emplace(k, SearchStateCache());
					cache.modify_if(k, [x,g](std::pair<const uint64_t,SearchStateCache>& kv){
						auto& v = kv.second;
						if (x<g) v.lo = std::max(v.lo, x);
						else v.hi = std::min(v.hi, x);
					});

					if (!s.parent) {
						out_move_i=s.best_move_i;
						out=s.best;
						return;
					}

					x*=-1;

					std::unique_lock lck2(s.parent->mut);
					if (s.move_i!=-1 && x>s.parent->best) {
						s.parent->best=x;
						s.parent->best_move_i=s.move_i;

						int g_parent = s.pos.next_player==init_player ? 1-gamma : gamma;
						if (x>g_parent) {
							s.parent->kill_children.notify_all();
						}
					}

					if (--s.parent->n_wait <= 0) s.parent->wait.notify_all();
				};

				bool ex = s.parent && s.parent->kill_children.test();
				if (ex) s.kill_children.notify_all();
				while (s.n_wait>0) s.wait.wait(self_lock);

				if (ex) {
					std::unique_lock lck2(s.parent->mut);
					if (--s.parent->n_wait <= 0) s.parent->wait.notify_all();
					continue;
				}

				if (s.visited) {
					update_parent(s.best);
					continue;
				}

				std::optional<int> upd;
				uint64_t k = s.hash^depth_hash[MIN_DEPTH+s.depth];

				cache.if_contains(k, [g,&upd](std::pair<const uint64_t,SearchStateCache> const& kv){
					if (kv.second.lo>=g) upd=kv.second.lo;
					else if (kv.second.hi<g) upd=kv.second.hi;
				});

				if (upd) {
					update_parent(*upd);
					continue;
				}

				if ((s.depth<=0 && s.score>=g) || s.depth<=-MIN_DEPTH || s.pos_ty!=PosType::Other) {
					update_parent(s.score);
					continue;
				}

				auto it2 = killer_move.find(s.hash);
				if (it2==killer_move.end() && s.depth > 2) {
					s.n_wait=1;

					std::unique_ptr<SearchState> new_state = std::make_unique<SearchState>(
						s.pos_ty, s.pos, s.hash,
						s.score, -1, s.depth-3, state.get()
					);

					lock.lock();

					stack.push_back(std::move(state));
					stack.push_back(std::move(new_state));

					lock.unlock();
					continue;
				}
				
				moves.clear();
				lua.valid_moves(moves, s.pos);
				n_calls++;

				tmp.clear();
				int n_move=0;
				assert(!moves.empty());
				for (auto& move: moves) {
					change(s, move);

					auto pos_type = lua.get_pos_type(s.pos);
					n_calls++;
					auto& new_state = tmp.emplace_back(
						n_move, std::make_unique<SearchState>(
							pos_type, s.pos, s.hash,
							s.score, n_move, s.depth-1, state.get()
						)
					).second;
						
					if (pos_type==PosType::Win) new_state->score = WINNING;
					else if (pos_type==PosType::Loss) new_state->score = LOSING;
					else if (pos_type==PosType::Draw) new_state->score = 0;

					change(s, move);

					n_move++;
				}

				auto tmp_it = tmp.begin();
				if (it2!=killer_move.end()) {
					swap(tmp[it2->second], tmp[0]);
					tmp_it++;
				}

				using TmpV = std::pair<int,std::unique_ptr<SearchState>>;
				std::sort(tmp_it, tmp.end(),
					[](TmpV const& a, TmpV const& b) {
						return b.second->score < a.second->score;
					}
				);

				int min_score = s.score + QS - QS_A*s.depth;

				lock.lock();
				stack.push_back(std::move(state));
				s.visited=true;
				s.n_wait=0;

				bool got_move=false;
				for (auto& new_state: tmp) {
					if (got_move && new_state.second->score < min_score) break;
					got_move=true;

					if (s.depth<=1 && new_state.second->score < g) {
						s.best = new_state.second->score;
						s.best_move_i = new_state.first;
						break;
					}

					new_state.second->move_i = new_state.first;

					s.n_wait++;
					stack.push_back(std::move(new_state.second));
				}

				lock.unlock();
			}
		}, pool.threads.size()+1);

		assert(out_move_i!=-1);
		return std::make_optional<std::pair<Move,int>>({init_moves[out_move_i], out});
	}

	static constexpr int TIME_LIMIT = 10000;
	std::optional<Move> search() {
		auto start = std::chrono::steady_clock::now();
		std::optional<Move> best;
		bool tle=false;

		for (int depth=1; !tle && depth<=max_depth; depth++) {
			std::cout<<"depth "<<depth<<", cache size "<<cache.size()<<", lua calls "<<n_calls<<std::endl;

			auto now = std::chrono::steady_clock::now();

			std::optional<Move> best2;
			int lo=LOSING, hi=WINNING;
			while (!tle && hi-lo > EVAL_ROUGHNESS) {
				int mid = (hi+lo+1)/2;
				auto ret = bound(mid, depth);
				if (!ret) return std::nullopt;

				if (ret->second >= mid) lo=mid; else hi=mid-1;
				best2=ret->first;

				tle|=std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count() > TIME_LIMIT;
			}

			best=best2;
		}

		return best;
	}
};