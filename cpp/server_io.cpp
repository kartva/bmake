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
    cout << coord.i << " " << coord.j;
}

// Receives a piece from the client
Piece receivePiece() {
    Piece piece;
    cin >> piece.type;
    piece.c = receiveCoord();
    cin >> piece.being_added;

    return piece;
}

void sendPiece(const Piece& piece) {
    cout << piece.type << " ";
    sendCoord(piece.c);
    cout << " " << piece.being_added << "\n";
}

// Main I/O functions
void intro(LuaInterface& lua) {
    auto [width, height] = lua.board_dims();
    Position pos = lua.initial_position();
    auto pieceNames = lua.piece_names();
    
    cout << width << " " << height << "\n" << pos.board.size() << "\n";
    for (const auto& piece : pos.board) {
        sendPiece(piece);
    }
    cout << pieceNames.size() << "\n";
    for (const auto& [type, name] : pieceNames) {
        cout << type << " " << name << "\n";
    }
}

void sendMove(const Move& move) {
    sendCoord(move.from);
    cout << " ";
    sendCoord(move.to);
    cout << "\n" << move.add_remove.size() << "\n";
    
    for (const auto& piece : move.add_remove) {
        sendPiece(piece);
    }
}

void sendListOfMoves(const vector<Move>& moves) {
    cout << moves.size() << "\n";
    for (const auto& move : moves) {
        sendMove(move);
    }
}

Move receiveMove() {
    Move move;
    move.from = receiveCoord();
    move.to = receiveCoord();

    int add_remove_size;
    cin >> add_remove_size;
    for (int i = 0; i < add_remove_size; i++) {
        move.add_remove.push_back(receivePiece());
    }
    
    return move;
}