#include <iostream>
#include <map>
#include "lua_interface.hpp"
#include "util.hpp"

using namespace std;

Coord receiveCoord() {
    Coord coord;
    cin >> coord.i >> coord.j;
    return coord;
}

void sendCoord(const Coord& coord) {
    cout << coord.i << " " << coord.j << "\n";
}


void sendBoard(const unsigned char board[MAX_BOARD_SIZE]) {
    for (int i = 0; i < MAX_BOARD_SIZE - 1; i++) {
        cout << (int)board[i] << " ";
    }
    cout << (int)board[MAX_BOARD_SIZE - 1] << "\n";
}

void receiveBoard(unsigned char board[MAX_BOARD_SIZE]) {
    for (int i = 0; i < MAX_BOARD_SIZE; i++) {
        int val;
        cin >> val;
        board[i] = (unsigned char)val;
    }
}

void sendPosition(const Position& pos) {
    cout << pos.next_player << "\n";
    sendBoard(pos.board);
}

void receivePosition(Position& pos) {
    cin >> pos.next_player;
    receiveBoard(pos.board);
}

void sendMove(const Move& move) {
    sendCoord(move.from);
    sendCoord(move.to);
    sendBoard(move.board);
}

Move receiveMove() {
    Move move;
    move.from = receiveCoord();
    move.to = receiveCoord();
    receiveBoard(move.board);
    return move;
}

// Main I/O functions
void intro(LuaInterface& lua) {
    auto [width, height] = lua.board_dims();
    Position pos = lua.initial_position();
    auto pieceNames = lua.piece_names();
    
    // Send actual dimensions first
    cout << width << " " << height << "\n";
    // Then send the position using the fixed array
    sendPosition(pos);
    cout << pieceNames.size() << "\n";
    for (const auto& [type, name] : pieceNames) {
        cout << type << " " << name << "\n";
    }
}