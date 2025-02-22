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

struct LuaInterface {
	lua_State* L;

	LuaInterface(std::string const& path);
	~LuaInterface();
	PosMoves getValidMoves(int player, Position const& position);
	void checkLua(int r);
};