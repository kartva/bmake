#include <lua.hpp>

#include <lua.h>
#include <vector>
#include <tuple>
#include <stdexcept>

class ChessEngine {
private:
    lua_State* L;

    // Helper to check if Lua call was successful
    void checkLua(int r) {
        if (r != LUA_OK) {
            std::string errMsg = lua_tostring(L, -1);
            lua_pop(L, 1);
            throw std::runtime_error("Lua error: " + errMsg);
        }
    }

    // Convert C++ position to Lua table
    void pushPosition(const std::vector<std::tuple<std::pair<int, int>, std::pair<int, int>>>& position) {
        lua_createtable(L, position.size(), 0);
        int index = 1;
        
        for (const auto& piece : position) {
            lua_createtable(L, 2, 0);
            
            // Push piece type and color
            lua_createtable(L, 2, 0);
            lua_pushinteger(L, std::get<0>(piece).first);
            lua_rawseti(L, -2, 1);
            lua_pushinteger(L, std::get<0>(piece).second);
            lua_rawseti(L, -2, 2);
            lua_rawseti(L, -2, 1);
            
            // Push position coordinates
            lua_createtable(L, 2, 0);
            lua_pushinteger(L, std::get<1>(piece).first);
            lua_rawseti(L, -2, 1);
            lua_pushinteger(L, std::get<1>(piece).second);
            lua_rawseti(L, -2, 2);
            lua_rawseti(L, -2, 2);
            
            lua_rawseti(L, -2, index++);
        }
    }

    // Convert Lua moves table to C++ vector
    std::vector<std::vector<std::tuple<std::string, std::pair<int, int>, std::pair<int, int>>>> 
    parseMoves() {
        std::vector<std::vector<std::tuple<std::string, std::pair<int, int>, std::pair<int, int>>>> moves;
        
        int numMoves = lua_rawlen(L, -1);
        for (int i = 1; i <= numMoves; i++) {
            lua_rawgeti(L, -1, i);
            std::vector<std::tuple<std::string, std::pair<int, int>, std::pair<int, int>>> move;
            
            int numOperations = lua_rawlen(L, -1);
            for (int j = 1; j <= numOperations; j++) {
                lua_rawgeti(L, -1, j);
                
                // Get operation type
                lua_rawgeti(L, -1, 1);
                std::string op = lua_tostring(L, -1);
                lua_pop(L, 1);
                
                // Get piece info
                lua_rawgeti(L, -1, 2);
                lua_rawgeti(L, -1, 1);
                int pieceType = lua_tointeger(L, -1);
                lua_pop(L, 1);
                lua_rawgeti(L, -1, 2);
                int pieceColor = lua_tointeger(L, -1);
                lua_pop(L, 2);
                
                // Get coordinates
                lua_rawgeti(L, -1, 3);
                lua_rawgeti(L, -1, 1);
                int x = lua_tointeger(L, -1);
                lua_pop(L, 1);
                lua_rawgeti(L, -1, 2);
                int y = lua_tointeger(L, -1);
                lua_pop(L, 2);
                
                move.push_back(std::make_tuple(op, std::make_pair(pieceType, pieceColor), std::make_pair(x, y)));
                lua_pop(L, 1);
            }
            moves.push_back(move);
            lua_pop(L, 1);
        }
        return moves;
    }

public:
    ChessEngine() {
        L = luaL_newstate();
        luaL_openlibs(L);
        checkLua(luaL_dofile(L, "/bmake/lua-scripts/chess/specification.lua"));
    }

    ~ChessEngine() {
        lua_close(L);
    }

    std::vector<std::vector<std::tuple<std::string, std::pair<int, int>, std::pair<int, int>>>>
    getValidMoves(int player, const std::vector<std::tuple<std::pair<int, int>, std::pair<int, int>>>& position) {
        lua_getglobal(L, "moves");
        lua_pushinteger(L, player);
        pushPosition(position);
        
        checkLua(lua_pcall(L, 2, 1, 0));
        
        auto moves = parseMoves();
        lua_pop(L, 1);
        return moves;
    }
};
