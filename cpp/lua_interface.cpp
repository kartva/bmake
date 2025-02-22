#include "lua_interface.hpp"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include <stdexcept>
#include <vector>
#include <string>

void LuaInterface::checkLua(int r) {
	if (r != LUA_OK) {
		std::string errMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw std::runtime_error("Lua error: " + errMsg);
	}
}

LuaInterface::LuaInterface(std::string const& path) {
	L = luaL_newstate();
	luaL_openlibs(L);
	checkLua(luaL_dofile(L, path.c_str()));
}

LuaInterface::~LuaInterface() {
	lua_close(L);
}

PosMoves LuaInterface::getValidMoves(Position const& position) {
	lua_getglobal(L, "moves"); // stack: moves()
	lua_pushinteger(L, position.player); // stack: moves, player

	lua_createtable(L, position.board.size(), 0); // stack: moves, player, board
	int index = 1;
	
	// For every piece on the board, add it to the board table
	for (auto p: position.board) {
		// stack: moves, player, board
		lua_createtable(L, 0, 2);  // stack: ..., piece and pos tables
		
		// Create piece table
		lua_pushstring(L, "piece");
		lua_createtable(L, 0, 1);
		lua_pushstring(L, "type");
		lua_pushinteger(L, p.type);
		lua_settable(L, -3);
		lua_settable(L, -3);
		
		// Create pos table
		lua_pushstring(L, "pos");
		lua_createtable(L, 0, 2);
		lua_pushstring(L, "x");
		lua_pushinteger(L, p.c.i);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushinteger(L, p.c.j);
		lua_settable(L, -3);
		lua_settable(L, -3);
		
		lua_rawseti(L, -2, index++);
	}
	
	//moves, player, board, piece
	checkLua(lua_pcall(L, 2, 1, 0));

	PosMoves out;
	if (lua_isboolean(L, -1)) {
		out.ty = lua_toboolean(L, -1) ? PosType::Win : PosType::Loss;
		lua_pop(L, 1);
		return out;
	}

	if (!lua_istable(L, -1)) throw std::runtime_error("return not a table");

	auto get_coord = [L=L]() {
		Coord c;
		lua_pushstring(L, "x");
		lua_gettable(L, -2);
		c.i=lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "y");
		lua_gettable(L, -2);
		c.j=lua_tointeger(L, -1);
		lua_pop(L, 1);

		return c;
	};

	int numMoves = lua_rawlen(L, -1);
	for (int i = 1; i <= numMoves; i++) {
		lua_rawgeti(L, -1, i);
		Move& move = out.moves.emplace_back();

		int numOperations = lua_rawlen(L, -1);
		for (int j = 1; j <= numOperations; j++) {
			lua_rawgeti(L, -1, j);
			
			lua_pushstring(L, "from");
			lua_gettable(L, -2);
			move.from = get_coord();
			lua_pop(L, 1);

			lua_pushstring(L, "to");
			lua_gettable(L, -2);
			move.to = get_coord();
			lua_pop(L, 1);

			lua_pushstring(L, "pieces");
			lua_gettable(L, -2);

			int numPieces = lua_rawlen(L, -1);
			for (int l=1; l<=numPieces; l++) {
				lua_rawgeti(L, -1, l);
				Piece& p = move.add_remove.emplace_back();
				p.c = get_coord();
				lua_pushstring(L, "type");
				lua_gettable(L, -2);
				p.type = lua_tointeger(L, -1);
				lua_pushstring(L, "add");
				lua_gettable(L, -2);
				p.being_added = lua_toboolean(L, -1);
				lua_pop(L, 1);
			}
			
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	return out;
}

Position LuaInterface::getInitialPosition() {
	Position pos;
	pos.player = 0; // make customizable?
	lua_getglobal(L, "initial_board");  // Stack: [initial_board]
	
	if (!lua_istable(L, -1)) {
			throw std::runtime_error("initial_board is not a table");
	}
	
	int numPieces = lua_rawlen(L, -1);
	for (int i = 1; i <= numPieces; i++) {
			lua_rawgeti(L, -1, i);  // Stack: [initial_board, piece]
			
			Piece p;
			
			// Get piece type
			lua_pushstring(L, "piece");  // Stack: [initial_board, piece, "piece"]
			lua_gettable(L, -2);        // Stack: [initial_board, piece, piece_table]
			lua_pushstring(L, "type");   // Stack: [initial_board, piece, piece_table, "type"]
			lua_gettable(L, -2);        // Stack: [initial_board, piece, piece_table, type]
			p.type = lua_tointeger(L, -1);
			lua_pop(L, 2);              // Stack: [initial_board, piece]
			
			// Get position
			lua_pushstring(L, "pos");    // Stack: [initial_board, piece, "pos"]
			lua_gettable(L, -2);        // Stack: [initial_board, piece, pos]
			lua_pushstring(L, "x");      // Stack: [initial_board, piece, pos, "x"]
			lua_gettable(L, -2);        // Stack: [initial_board, piece, pos, x]
			p.c.i = lua_tointeger(L, -1);
			lua_pop(L, 1);              // Stack: [initial_board, piece, pos]
			lua_pushstring(L, "y");      // Stack: [initial_board, piece, pos, "y"]
			lua_gettable(L, -2);        // Stack: [initial_board, piece, pos, y]
			p.c.j = lua_tointeger(L, -1);
			lua_pop(L, 2);              // Stack: [initial_board, piece]
			
			pos.board.push_back(p);
			lua_pop(L, 1);              // Stack: [initial_board]
	}
	
	lua_pop(L, 1);                  // Stack: []
	return pos;
}

std::unordered_map<int, std::string> LuaInterface::getPieceNames() {
    std::unordered_map<int, std::string> names;
    lua_getglobal(L, "piece_names"); // Stack: [piece_names]
    
    if (!lua_istable(L, -1)) {
        throw std::runtime_error("piece_names is not a table");
    }
    
    lua_pushnil(L);  // Stack: [piece_names, nil]
    while (lua_next(L, -2)) {  // Stack: [piece_names, key, value]
        int piece_type = lua_tointeger(L, -2);
        std::string name = lua_tostring(L, -1);
        names[piece_type] = name;
        lua_pop(L, 1);  // Stack: [piece_names, key]
    }
    
    lua_pop(L, 1);  // Stack: []
    return names;
}
