#include "util.hpp"
#include "lua_interface.hpp"
#include "server_io.hpp"
#include "nn.hpp"
#include "search.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	std::string lua = "/bmake/lua-scripts/chess/chess2.lua";

	auto interface = LuaInterface(lua);
	interface.validate();

	vec<Move> moves;
	interface.valid_moves(moves, interface.initial_position());

	for (const auto& move : moves) {
		std::cout << format("{}, {} to {}, {}", move.from.i, move.from.j, move.to.i, move.to.j) << std::endl;
	}
}