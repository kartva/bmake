#include "lua_interface.hpp"
#include "search.hpp"

#ifndef BUILD_DEBUG
#include <mimalloc-new-delete.h>
#endif

using namespace std;

int main() {
	std::string lua = "/bmake/lua-scripts/chess/chess2.lua";

	auto interface = LuaInterface(lua);
	interface.validate();

	auto init = interface.initial_position();
	Searcher searcher(
		12, 8, 8, 100, 0, lua, init
	);

	auto out = searcher.search();
	return 0;
}