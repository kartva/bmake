#include "lua_interface.hpp"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include <format>
#include <stdexcept>
#include <string>

struct LuaException: public std::exception {
	std::string err;
	LuaException(std::string const& err): err(err) {}
	char const* what() const noexcept { return err.c_str(); }
};

void LuaInterface::check(int r) {
	if (r != LUA_OK) {
		std::string errMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw LuaException(std::format("Lua Runtime Error:\n{}", errMsg));
	}
}

LuaInterface::LuaInterface(std::string const& path) {
	L = luaL_newstate();
	luaL_openlibs(L);
	check(luaL_dofile(L, path.c_str()));
}

LuaInterface::~LuaInterface() {
	lua_close(L);
}

void LuaInterface::push_position(Position const& position) {
	lua_pushinteger(L, position.player+1); // stack: moves, player

	lua_createtable(L, position.board.size(), 0); // stack: moves, player, board
	int index = 1;
	
	// For every piece on the board, add it to the board table
	for (auto p: position.board) {
		// stack: moves, player, board
		lua_createtable(L, 0, 2);  // stack: moves, player, board, piece and pos tables
		
		// Create piece table
		lua_pushstring(L, "piece"); // stack: moves, player, board, piece and pos tables, "piece"
		lua_createtable(L, 0, 1); // stack: moves, player, board, piece and pos tables, "piece", piece_table
		lua_pushstring(L, "type"); // stack: moves, player, board, piece and pos tables, "piece", piece_table, "type"
		lua_pushinteger(L, p.type); // stack: moves, player, board, piece and pos tables, "piece", piece_table, "type", type_value
		lua_settable(L, -3); // stack: moves, player, board, piece and pos tables, "piece", piece_table
		lua_settable(L, -3); // stack: moves, player, board, piece and pos tables
		
		// Create pos table
		lua_pushstring(L, "pos"); // stack: moves, player, board, piece and pos tables, "pos"
		lua_createtable(L, 0, 2); // stack: moves, player, board, piece and pos tables, "pos", pos_table
		lua_pushstring(L, "x"); // stack: moves, player, board, piece and pos tables, "pos", pos_table, "x"
		lua_pushinteger(L, p.c.i); // stack: moves, player, board, piece and pos tables, "pos", pos_table, "x", x_value
		lua_settable(L, -3); // stack: moves, player, board, piece and pos tables, "pos", pos_table
		lua_pushstring(L, "y"); // stack: moves, player, board, piece and pos tables, "pos", pos_table, "y"
		lua_pushinteger(L, p.c.j); // stack: moves, player, board, piece and pos tables, "pos", pos_table, "y", y_value
		lua_settable(L, -3); // stack: moves, player, board, piece and pos tables, "pos", pos_table
		lua_settable(L, -3); // stack: moves, player, board, piece and pos tables
		
		lua_rawseti(L, -2, index++); // stack: moves, player, board
	}
}

PosType LuaInterface::get_pos_type(Position const& position) {
	lua_getglobal(L, "type"); // stack: moves()
	push_position(position);
	check(lua_pcall(L, 2, 1, 0)); // stack: result

	PosType ret;
	if (lua_isnil(L, -1)) {
		ret=PosType::Other;
	} else {
		char const* ret_str = lua_tostring(L, -1); // stack: result

		if (!ret_str) throw LuaException("Position type not a string");

		if (strcmp(ret_str, "win")==0) ret=PosType::Win;
		else if (strcmp(ret_str, "draw")==0) ret=PosType::Draw;
		else if (strcmp(ret_str, "loss")==0) ret=PosType::Loss;
		else throw LuaException("Unrecognized position outcome");
	}

	lua_pop(L, 1);
	return ret;
}

void LuaInterface::valid_moves(vec<Move>& out, Position const& position) {
	lua_getglobal(L, "moves"); // stack: moves()
	push_position(position);
	
	//moves, player, board, piece
	check(lua_pcall(L, 2, 1, 0)); // stack: result

	if (!lua_istable(L, -1)) throw LuaException("Return is not a table");

	auto get_coord = [L=L]() {
		Coord c;
		lua_pushstring(L, "x"); // stack: ..., "x"
		lua_gettable(L, -2); // stack: ..., x_value
		c.i=lua_tointeger(L, -1); // stack: ..., x_value
		lua_pop(L, 1); // stack: ...

		lua_pushstring(L, "y"); // stack: ..., "y"
		lua_gettable(L, -2); // stack: ..., y_value
		c.j=lua_tointeger(L, -1); // stack: ..., y_value
		lua_pop(L, 1); // stack: ...

		return c;
	};

	int numMoves = lua_rawlen(L, -1); // stack: result
	out.reserve(numMoves);

	for (int i = 1; i <= numMoves; i++) {
		lua_rawgeti(L, -1, i); // stack: result, move
		Move& move = out.emplace_back();

		int numOperations = lua_rawlen(L, -1); // stack: result, move
		for (int j = 1; j <= numOperations; j++) {
			lua_rawgeti(L, -1, j); // stack: result, move, operation
			
			lua_pushstring(L, "from"); // stack: result, move, operation, "from"
			lua_gettable(L, -2); // stack: result, move, operation, from_coord
			move.from = get_coord(); // stack: result, move, operation, from_coord
			lua_pop(L, 1); // stack: result, move, operation

			lua_pushstring(L, "to"); // stack: result, move, operation, "to"
			lua_gettable(L, -2); // stack: result, move, operation, to_coord
			move.to = get_coord(); // stack: result, move, operation, to_coord
			lua_pop(L, 1); // stack: result, move, operation

			lua_pushstring(L, "pieces"); // stack: result, move, operation, "pieces"
			lua_gettable(L, -2); // stack: result, move, operation, pieces_table

			int numPieces = lua_rawlen(L, -1); // stack: result, move, operation, pieces_table
			for (int l=1; l<=numPieces; l++) {
				lua_rawgeti(L, -1, l); // stack: result, move, operation, pieces_table, piece
				Piece& p = move.add_remove.emplace_back();
				p.c = get_coord(); // stack: result, move, operation, pieces_table, piece
				lua_pushstring(L, "type"); // stack: result, move, operation, pieces_table, piece, "type"
				lua_gettable(L, -2); // stack: result, move, operation, pieces_table, piece, type_value
				p.type = lua_tointeger(L, -1); // stack: result, move, operation, pieces_table, piece, type_value
				lua_pop(L, 1); // stack: result, move, operation, pieces_table, piece
				lua_pushstring(L, "add"); // stack: result, move, operation, pieces_table, piece, "add"
				lua_gettable(L, -2); // stack: result, move, operation, pieces_table, piece, add_value
				p.being_added = lua_toboolean(L, -1); // stack: result, move, operation, pieces_table, piece, add_value
				lua_pop(L, 1); // stack: result, move, operation, pieces_table, piece
				lua_pop(L, 1); // stack: result, move, operation, pieces_table
			}
			
			lua_pop(L, 1); // stack: result, move, operation
		}

		lua_pop(L, 1); // stack: result, move
	}

	lua_pop(L, 1); // stack: result
}

Position LuaInterface::initial_position() {
	Position pos;
	pos.player = 0; // make customizable?
	lua_getglobal(L, "initial_board");  // Stack: [initial_board]
	
	if (!lua_istable(L, -1)) {
		throw LuaException("initial_board is not a table");
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

std::unordered_map<int, std::string> LuaInterface::piece_names() {
    std::unordered_map<int, std::string> names;
    lua_getglobal(L, "piece_names"); // Stack: [piece_names]
    
    if (!lua_istable(L, -1)) {
        throw LuaException("piece_names is not a table");
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

std::pair<int, int> LuaInterface::board_dims() {
    lua_getglobal(L, "BOARD_WIDTH");
    lua_getglobal(L, "BOARD_HEIGHT");
    
    if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
        throw LuaException("Board dimensions not properly defined");
    }
    
    int width = lua_tointeger(L, -2);
    int height = lua_tointeger(L, -1);
    lua_pop(L, 2);
    
    return {width, height};
}

void LuaInterface::validate() {
	auto init = initial_position();
	vec<Move> moves;
	valid_moves(moves, init);
	get_pos_type(init);
}
