#include <iostream>
#include <vector>
#include <string>

#include "lua.hpp"
#include "util.hpp"

using namespace std;

// Transmits the board dimensions, number of pieces, and the pieces and the start position
void intro(int width, int height, int numPieces, vector<Piece>& pieces) {
    cout << width << " " << height << " " << numPieces << "\n";
    for (int i = 0; i < numPieces; i++) {
        cout << pieces[i].type << " " << pieces[i].x << " " << pieces[i].y << "\n";
    }
}

// 
Piece receivePiece() {
    Piece piece;
    cin >> piece.type >> " " >> piece.x >> " " >> piece.y >> "\n";
    return piece;
}

void sendListofMoves(int numMoves, vector<Move>& moves) {
    Piece piece = receivePiece();
    cout << numMoves << "\n";
    for (int i = 0; i < numMoves; i++) {
        cout << moves[i].x << " " << moves[i].y << "\n";
    }
}
    