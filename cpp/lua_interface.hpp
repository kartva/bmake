#pragma once

#include "util.hpp"
#include "lua.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

enum class PosType {
	Win, Loss, Other
};

struct PosMoves {
	PosType ty;
	vec<Move> moves;
};

struct LuaInterface {
	lua_State* L;

	LuaInterface(std::string const& path);
	~LuaInterface();
	PosMoves getValidMoves(Position const& position);
	void checkLua(int r);

	// Returns the initial board state
	Position getInitialPosition();
	
	// Returns mapping of piece numbers to their string representations
	std::unordered_map<int, std::string> getPieceNames();
};