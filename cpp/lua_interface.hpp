#pragma once

extern "C" {
	#include "lauxlib.h"
	#include "lua.h"
	#include "lualib.h"
}

#include "util.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>

enum class PosType {
	Win, Loss, Draw, Other
};

struct LuaInterface {
	lua_State* L;
	int n,m;

	LuaInterface(std::string const& path, int n, int m);
	~LuaInterface();

	void push_position(Position const& position);
	PosType get_pos_type(Position const& position);
	void valid_moves(vec<Move>& out, Position const& position);
	void check(int r);
	void validate();

	// Returns the initial board state
	Position initial_position();
	
	// Returns mapping of piece numbers to their string representations
	std::unordered_map<int, std::string> piece_names();

	// Returns the dimensions of the board
	std::pair<int, int> board_dims();
};