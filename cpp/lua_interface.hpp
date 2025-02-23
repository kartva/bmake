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

struct LuaException: public std::exception {
	std::string err;
	LuaException(std::string const& err): err(err) {}
	char const* what() const noexcept { return err.c_str(); }
};

struct LuaInterface {
	lua_State* L;
	int n,m; // Board dimensions found extracted from Lua

	LuaInterface(): L(nullptr) {}
	LuaInterface(std::string const& path);
	LuaInterface(LuaInterface& other) = delete;
	LuaInterface(LuaInterface&& other): L(other.L), n(other.n), m(other.m) {
		other.L = nullptr;
	}
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