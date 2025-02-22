#include "lua_interface.hpp"
#include "search.hpp"

#ifndef BUILD_DEBUG
#include <mimalloc-new-delete.h>
#endif

using namespace std;

int main() {
	std::string lua = "/bmake/lua-scripts/chess/specification.lua";

	auto interface = LuaInterface(lua);
	interface.validate();

	auto init = interface.initial_position();
	Searcher searcher(
		2, 1, 10, 100, 8, lua, init
	);

	auto out = searcher.search();
	return 0;
}