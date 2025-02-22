#pragma once

#include "util.hpp"
#include "lua.h"
#include <stdexcept>
#include <string>

enum class PosType {
	Win, Loss, Other
};

struct PosMoves {
	PosType ty;
	vec<Move> moves;
};

struct ChessEngine {
	lua_State* L;

	ChessEngine();
	~ChessEngine();
	PosMoves getValidMoves(int player, Position const& position);
	void checkLua(int r);
};