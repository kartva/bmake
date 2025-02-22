#include "lua_interface.hpp"
#include "util.hpp"

#include <format>
#include <stdexcept>
#include <string>

void LuaInterface::check(int r) {
	if (r != LUA_OK) {
		std::string errMsg = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw LuaException(std::format("Lua Runtime Error:\n{}", errMsg));
	}
}

struct LuaBoard {
	int n,m;
	unsigned char board[MAX_BOARD_SIZE];
};

int board_get(lua_State* L) { 
	LuaBoard* board = static_cast<LuaBoard*>(luaL_checkudata(L, lua_upvalueindex(1), "board"));
	int i = luaL_checkinteger(L, 1)-1;
	int j = luaL_checkinteger(L, 2)-1;

	if (j<0 || j>=board->m || i<0 || i>=board->n) lua_pushnil(L);
	else lua_pushinteger(L, board->board[i*board->m + j]);
	return 1; 
}

int board_set(lua_State* L) { 
	LuaBoard* board = static_cast<LuaBoard*>(luaL_checkudata(L, lua_upvalueindex(1), "board"));
	int i = luaL_checkinteger(L, 1)-1;
	int j = luaL_checkinteger(L, 2)-1;
	int v = luaL_checkinteger(L, 3);

	if (j<0 || j>=board->m || i<0 || i>=board->n) {
		return luaL_error(L, "index %i, %i out of bounds of %i x %i board", i,j, board->n, board->m);
	} else {
		board->board[i*board->m + j] = v;
	}

	return 0;
}

int board_clone(lua_State* L);

void push_board(lua_State* L, unsigned char const* b, int n, int m) {
	lua_createtable(L, 0, 3);

	auto board = static_cast<LuaBoard*>(lua_newuserdata(L, sizeof(LuaBoard)));
	board->n=n, board->m=m;
	std::copy(b,b+n*m, board->board);
	luaL_setmetatable(L, "board");

	lua_pushvalue(L, -1);
	lua_pushcclosure(L, board_get, 1);
	lua_setfield(L, -3, "get");

	lua_pushvalue(L, -1);
	lua_pushcclosure(L, board_set, 1);
	lua_setfield(L, -3, "set");

	lua_pushvalue(L, -1);
	lua_pushcclosure(L, board_clone, 1);
	lua_setfield(L, -3, "clone");

	lua_setfield(L, -2, "inner");
}

int board_clone(lua_State* L) {
	LuaBoard* old = static_cast<LuaBoard*>(luaL_checkudata(L, lua_upvalueindex(1), "board"));
	push_board(L, old->board, old->n, old->m);
	return 1;
}

LuaInterface::LuaInterface(std::string const& path) {
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_newmetatable(L, "board");

	lua_getglobal(L, "io");
	lua_pushstring(L, "stdout");
	lua_gettable(L, -2);

	((luaL_Stream *)lua_touserdata(L, -1))->f = stdout;

	lua_pop(L, 2);

	check(luaL_dofile(L, path.c_str()));

	lua_getglobal(L, "BOARD_WIDTH");
	lua_getglobal(L, "BOARD_HEIGHT");
	
	if (!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
		throw LuaException("Board dimensions not properly defined");
	}
	
	n = lua_tointeger(L, -2);
	m = lua_tointeger(L, -1);
	lua_pop(L, 2);
}

LuaInterface::~LuaInterface() {
	if (L) lua_close(L);
}

void LuaInterface::push_position(Position const& pos) {
	lua_pushinteger(L, pos.next_player+1); // stack: moves, player
	push_board(L, pos.board, n, m);
}

PosType LuaInterface::get_pos_type(Position const& position) {
	lua_getglobal(L, "Type"); // stack: moves()
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
	lua_getglobal(L, "Moves"); // stack: moves()
	push_position(position);
	
	//moves, player, board, piece
	check(lua_pcall(L, 2, 1, 0)); // stack: result
	
	if (!lua_istable(L, -1)) throw LuaException("Return is not a table");
	
	auto get_coord = [L=L]() {
		Coord c;
		if (lua_rawlen(L, -1)!=2) throw LuaException("expected 2 numbers for coord");

		lua_rawgeti(L, -1, 1);
		c.i=luaL_checkinteger(L, -1)-1; // stack: ..., x_value
		lua_pop(L, 1); // stack: ...
		
		lua_rawgeti(L, -1, 2);
		c.i=luaL_checkinteger(L, -1)-1; // stack: ..., x_value
		lua_pop(L, 1); // stack: ...

		return c;
	};
	
	int numMoves = lua_rawlen(L, -1); // stack: result
	out.reserve(numMoves);
	
	for (int i = 1; i <= numMoves; i++) {
		lua_rawgeti(L, -1, i); // stack: result, move
		Move& move = out.emplace_back();
		
		lua_getfield(L, -1, "from"); // stack: result, move, operation, from_coord
		move.from = get_coord(); // stack: result, move, operation, from_coord
		lua_pop(L, 1); // stack: result, move, operation
		
		lua_getfield(L, -1, "to"); // stack: result, move, operation, from_coord
		move.to = get_coord(); // stack: result, move, operation, to_coord
		lua_pop(L, 1); // stack: result, move, operation
			
		lua_getfield(L, -1, "board");
		lua_getfield(L, -1, "inner");

		auto b = static_cast<LuaBoard*>(luaL_checkudata(L, -1, "board"));
		std::copy(b->board, b->board + n*m, move.board);
			
		lua_pop(L, 3); //pop udata, board, move
	}
	
	lua_pop(L, 1); // stack: result
}

Position LuaInterface::initial_position() {
	Position pos;
	pos.next_player = 0; // make customizable?
	lua_getglobal(L, "InitialBoard");  // Stack: [initial_board]

	if (!lua_istable(L, -1)) {
		throw LuaException("initial_board is not a table");
	}
	
	if (lua_rawlen(L, -1)!=n) throw LuaException("Wrong # rows");

	for (int i=1; i<=n; i++) {
		lua_rawgeti(L, -1, i);  // Stack: [initial_board, piece]
		if (lua_rawlen(L, -1)!=m) throw LuaException("Wrong # cols");

		for (int j=1; j<=m; j++) {
			lua_rawgeti(L, -1, j);
			pos.board[(i-1)*m+j-1] = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
		}
		
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

// Returns width, height.
std::pair<int, int> LuaInterface::board_dims() {
	return {m, n};
}

void LuaInterface::validate() {
	auto init = initial_position();
	vec<Move> moves;
	valid_moves(moves, init);
	get_pos_type(init);
}
