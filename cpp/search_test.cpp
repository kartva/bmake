#include "lua_interface.hpp"
#include "search.hpp"
#include <bits/stdc++.h>
using namespace std;

int main() {
	std::string lua = "/bmake/lua-scripts/chess/specification.lua";
	auto leng = LuaInterface(lua, 1, 10);
	auto init = leng.initial_position();
	Searcher searcher(
		2, 1, 10, 10, 1, lua, init
	);

	auto out = searcher.search();
	return 0;
}