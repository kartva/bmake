#include "lua_interface.hpp"
#include "search2.hpp"

#ifndef BUILD_DEBUG
#include <mimalloc-new-delete.h>
#endif

using namespace std;

int main() {
	std::string lua = "/bmake/lua-scripts/chess/chess2.lua";

	auto interface = LuaInterface(lua);

	auto init = interface.initial_position();
	Searcher searcher(
		12, 8, 8, 100, 0, lua
	);

	auto out = searcher.search(init);
	return 0;
}