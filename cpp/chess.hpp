#pragma once

#include <vector>
#include <string>

#include "util.hpp"

constexpr int BOARD_WIDTH = 8;
constexpr int BOARD_HEIGHT = 8;

// Piece names: index 0 = empty, 1–6 for player 1, 7–12 for player 2.
extern const char piece_names[13];

struct MoveCandidate {
    unsigned char toRow;
    unsigned char toCol;
    bool promotion = false;
    bool castling = false;
};

std::vector<MoveCandidate> generateMovesCommon(unsigned char piece, int row, int col, const Position &position, bool attacking);
bool IsAttacked(const Position &position, int row, int col, int opponent);
bool IsInCheck(int player, const Position &position);

void addCastlingMoves(int piece, Position &position, std::vector<MoveCandidate>& moves);

std::vector<MoveCandidate> GenerateMoves(int piece, int row, int col, Position const&position);
std::vector<MoveCandidate> GenerateAttackingMoves(int piece, int row, int col, Position &position);
std::vector<Move> Moves(int player, Position const&position);
std::string Type(int player, Position &position);
void better_valid_moves(vec<Move>& out, Position const& position);



Position InitialBoard();
