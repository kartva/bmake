#include <vector>
#include <string>
#include <utility>
#include <iostream>

#include "chess.hpp"

// // Board dimensions.
// constexpr int BOARD_WIDTH = 8;
// constexpr int BOARD_HEIGHT = 8;

// Piece names (index 0 = empty, 1-6 for player 1, 7-12 for player 2).
const char piece_names[13] = {
    '.', 'P', 'N', 'B', 'R', 'Q', 'K','p', 'n', 'b', 'r', 'q', 'k'
};

// Board::Board() {
//     for (auto &row : squares)
//         for (int &cell : row)
//             cell = 0;
// }

// int Board::get(int row, int col) const {
//     return squares[row][col];
// }

// void Board::set(int row, int col, int value) {
//     squares[row][col] = value;
// }

// // clone() simply returns a copy of the board.
// Board Board::clone() const {
//     return *this;
// }

unsigned char get(const Position &pos, unsigned char i, unsigned char j) {
    return pos.board[i * 8 + j];
}

void set(Position &pos, unsigned char i, unsigned char j, unsigned char value) {
    pos.board[i * 8 + j] = value;
}

Position clone(const Position &pos) {
    Position new_pos;
    new_pos.next_player = pos.next_player;
    for (int i = 0; i < 64; i++) {
        new_pos.board[i] = pos.board[i];
    }
    return new_pos;
    // return pos;
}

inline bool belongsToPlayer(int piece, int player) {
    if (piece == 0)
        return false;
    if (player == 1)
        return (piece >= 1 && piece <= 6);
    else
        return (piece >= 7 && piece <= 12);
}

inline bool isOpponent(int piece, int player) {
    if (piece == 0)
        return false;
    return !belongsToPlayer(piece, player);
}

// std::vector<MoveCandidate> generateMovesCommon(int piece, int row, int col, Position &position, bool attacking);
// bool IsAttacked(Position &position, int row, int col, int opponent);
// bool IsInCheck(int player, Position &position);

void addCastlingMoves(unsigned char piece, const Position &position, std::vector<MoveCandidate>& moves) {
    if (piece != 6 && piece != 12)
        return;

    int player = (piece <= 6) ? 1 : 2;

    // For 0-indexing: white king row = 0, black king row = 7.
    int king_row = (player == 1) ? 0 : 7;
    int king_col = 4; // starting king column in our conversion
    int rook_cols[2] = {0, 7};

    for (int idx = 0; idx < 2; idx++) {
        int rook_col = rook_cols[idx];
        // Only proceed if a rook exists at its original square.
        if (get(position, king_row, rook_col) != 0) {
            bool clear_path = true;
            int step = (rook_col > king_col) ? 1 : -1;
            for (int col_iter = king_col + step; col_iter != rook_col; col_iter += step) {
                if (get(position, king_row, col_iter) != 0) {
                    clear_path = false;
                    break;
                }
            }
            if (clear_path) {
                // Check that the king's current square and the two squares it must cross
                // are not attacked.
                bool safe = true;
                int opponent = (player == 1) ? 2 : 1;
                for (int offset = 0; offset <= 2; offset++) {
                    int check_col = king_col + offset * step;
                    if (IsAttacked(position, king_row, check_col, opponent)) {
                        safe = false;
                        break;
                    }
                }
                if (safe && !IsInCheck(player, position)) {
                    MoveCandidate mc;
                    mc.toRow = king_row;
                    mc.toCol = king_col + 2 * step; // king moves two squares
                    mc.castling = true;
                    moves.push_back(mc);
                }
            }
        }
    }
}

//--------------------------------------------------------------
// Generate moves common to all pieces (pawn, knight, bishop, rook, queen, king).
// The 'attacking' flag is used to optionally avoid legality (check) filtering.
std::vector<MoveCandidate> generateMovesCommon(int piece, int row, int col, const Position &position, bool attacking) {
    std::vector<MoveCandidate> moves;
    int player = (piece <= 6) ? 1 : 2;
    int abs_piece = (piece <= 6) ? piece : piece - 6;

    if (abs_piece == 1) { // Pawn
        int dir = (player == 1) ? 1 : -1;
        if (attacking) {
            for (int dj : {-1, 1}) {
                int x = row + dir, y = col + dj;
                if (x >= 0 && x < BOARD_HEIGHT && y >= 0 && y < BOARD_WIDTH) {
                    MoveCandidate mc;
                    mc.toRow = x;
                    mc.toCol = y;
                    moves.push_back(mc);
                }
            }
        } else {
            int start_row = (player == 1) ? 1 : 6;
            int promo_row = (player == 1) ? 7 : 0;
            int x = row + dir;
            if (x >= 0 && x < BOARD_HEIGHT && get(position, x, col) == 0) {
                MoveCandidate mc;
                mc.toRow = x;
                mc.toCol = col;
                if (x == promo_row)
                    mc.promotion = true;
                moves.push_back(mc);
                if (row == start_row && get(position, row + 2 * dir, col) == 0) {
                    MoveCandidate mc2;
                    mc2.toRow = row + 2 * dir;
                    mc2.toCol = col;
                    moves.push_back(mc2);
                }
            }
            for (int dj : {-1, 1}) {
                int x = row + dir, y = col + dj;
                if (x >= 0 && x < BOARD_HEIGHT && y >= 0 && y < BOARD_WIDTH) {
                    int target = get(position, x, y);
                    if (target != 0 && isOpponent(target, player)) {
                        MoveCandidate mc;
                        mc.toRow = x;
                        mc.toCol = y;
                        if (x == promo_row)
                            mc.promotion = true;
                        moves.push_back(mc);
                    }
                }
            }
        }
    } else {
        // Define directions for knights, bishops, rooks, queens, and kings.
        // Note that knights (abs_piece == 2) and kings (abs_piece == 6) do not slide.
        std::vector<std::vector<std::pair<int,int>>> directions(7);
        directions[2] = { {-2,-1}, {-1,-2}, {1,-2}, {2,-1}, {2,1}, {1,2}, {-1,2}, {-2,1} };   // Knight
        directions[3] = { {-1,-1}, {-1,1}, {1,-1}, {1,1} };                                     // Bishop
        directions[4] = { {-1,0}, {1,0}, {0,-1}, {0,1} };                                         // Rook
        directions[5] = { {-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1} };         // Queen
        directions[6] = { {-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1} };         // King

        if (!directions[abs_piece].empty()) {
            for (auto d : directions[abs_piece]) {
                int x = row, y = col;
                while (true) {
                    x += d.first;
                    y += d.second;
                    if (x < 0 || x >= BOARD_HEIGHT || y < 0 || y >= BOARD_WIDTH)
                        break;
                    int target = get(position, x, y);
                    if (target != 0) {
                        if (isOpponent(target, player)) {
                            MoveCandidate mc;
                            mc.toRow = x;
                            mc.toCol = y;
                            moves.push_back(mc);
                        }
                        break;
                    }
                    MoveCandidate mc;
                    mc.toRow = x;
                    mc.toCol = y;
                    moves.push_back(mc);
                    if (abs_piece == 2 || abs_piece == 6)  // knights and kings move only one step
                        break;
                }
            }
        }
    }

    if (!attacking && abs_piece == 6) {
        addCastlingMoves(piece, position, moves);
    }

    if (attacking)
        return moves;

    Position pclone = clone(position);

    // Remove moves that lead to check.
    std::vector<MoveCandidate> legalMoves;
    for (auto &move : moves) {
        int oldPiece = get(position, move.toRow, move.toCol);
        set(pclone, row, col, 0);
        set(pclone, move.toRow, move.toCol, piece);
        bool inCheck = IsInCheck(player, pclone);
        // revert board changes
        set(pclone, row, col, piece);
        set(pclone, move.toRow, move.toCol, oldPiece);
        if (!inCheck)
            legalMoves.push_back(move);
    }
    return legalMoves;
}

//--------------------------------------------------------------
// Check if a square is attacked by any piece of the given opponent.
bool IsAttacked(const Position &position, int row, int col, int opponent) {
    for (int x = 0; x < BOARD_HEIGHT; x++) {
        for (int y = 0; y < BOARD_WIDTH; y++) {
            int piece = get(position, x, y);
            if (belongsToPlayer(piece, opponent)) {
                auto attacking = generateMovesCommon(piece, x, y, position, true);
                for (auto &move : attacking) {
                    if (move.toRow == row && move.toCol == col)
                        return true;
                }
            }
        }
    }
    return false;
}

//--------------------------------------------------------------
// Check if the player's king is in check.
bool IsInCheck(int player, const Position &position) {
    int king = (player == 1) ? 6 : 12;
    std::pair<int,int> king_pos = {-1, -1};
    for (unsigned char i = 0; i < BOARD_HEIGHT; i++) {
        for (unsigned char j = 0; j < BOARD_WIDTH; j++) {
            if (get(position, i, j) == king) {
                king_pos = {i, j};
                break;
            }
        }
        if (king_pos.first != -1)
            break;
    }
    if (king_pos.first == -1)
        return true; // King "captured"
    int opponent = (player == 1) ? 2 : 1;
    return IsAttacked(position, king_pos.first, king_pos.second, opponent);
}

//--------------------------------------------------------------
// Public move generators.
std::vector<MoveCandidate> GenerateMoves(int piece, int row, int col, const Position &position) {
    return generateMovesCommon(piece, row, col, position, false);
}

std::vector<MoveCandidate> GenerateAttackingMoves(int piece, int row, int col, const Position &position) {
    return generateMovesCommon(piece, row, col, position, true);
}

std::vector<Move> Moves(int player, const Position &position) {
    std::vector<Move> out;
    for (unsigned char i = 0; i < BOARD_HEIGHT; i++) {
        for (unsigned char j = 0; j < BOARD_WIDTH; j++) {
            int piece = get(position, i, j);
            if (belongsToPlayer(piece, player)) {
                auto pieceMoves = GenerateMoves(piece, i, j, position);
                for (auto &move : pieceMoves) {
                    Position new_board = clone(position);
                    set(new_board, i, j, 0);
                    int movingPiece = piece;
                    if (move.promotion) {
                        // Promote pawn to queen.
                        movingPiece = 6 * (player - 1) + 5;
                    }
                    if (move.castling) {
                        // For castling: in our conversion, if king moves to col 2, it is queen‐side;
                        // if king moves to col 6, it is king‐side.
                        int rook_col = (move.toCol == 2) ? 0 : 7;
                        int rook = 6 * (player - 1) + 4;
                        set(new_board, i, rook_col, 0);
                        // Queen‐side: rook moves to col 3; king‐side: rook moves to col 5.
                        set(new_board, i, (move.toCol == 2) ? 3 : 5, rook);
                    }
                    set(new_board, move.toRow, move.toCol, movingPiece);
                    Move m;
                    m.from = {i, j};
                    m.to = {move.toRow, move.toCol};
                    // m.board = new_board.board;
                    memcpy(m.board, new_board.board, sizeof(new_board.board));
                    out.push_back(m);
                }
            }
        }
    }
    return out;
}

std::string Type(int player, Position &position) {
    auto moves = Moves(player, position);
    if (moves.empty())
        return IsInCheck(player, position) ? "loss" : "draw";
    
    int opponent = (player == 1) ? 2 : 1;
    auto moves_opp = Moves(opponent, position);
    if (moves_opp.empty())
        return IsInCheck(opponent, position) ? "win" : "draw";
    
    return "";
}

Position InitialBoard() {
    Position board;
    int init[BOARD_HEIGHT][BOARD_WIDTH] = {
        {4, 2, 3, 5, 6, 3, 2, 4},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {7, 7, 7, 7, 7, 7, 7, 7},
        {10, 8, 9, 11, 12, 9, 8, 10}

        // {6, 0, 0, 4, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 3, 0, 0, 0, 0, 0},
        // {0, 0, 0, 10, 0, 0, 0, 0},
        // {0, 0, 0, 2, 0, 0, 0, 5},
        // {0, 0, 11, 0, 0, 12, 0, 0}

        // {6, 0, 0, 4, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 0, 0, 0, 0, 0, 0},
        // {0, 0, 3, 0, 0, 0, 0, 0},
        // {0, 0, 0, 10, 0, 0, 0, 0},
        // {0, 0, 0, 2, 0, 0, 0, 5},
        // {0, 0, 11, 0, 0, 12, 0, 0}
    };
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            set(board, i, j, init[i][j]);
        }
    }
    return board;
}

void better_valid_moves(vec<Move>& out, Position const& position) {
    int curr_player = position.next_player == 1 ? 2 : 1;
    auto moves = Moves(curr_player, position);
    for (auto &move : moves) {
        // std::cout << (int) move.from.i << "," << (int) move.from.j << " : " << (int) move.to.i << "," << (int) move.to.j << " " << piece_names[move.board[8 * move.to.i + move.to.j]] << '\n';
        out.push_back(move);
    }

    // std::cerr << "Valid moves: " << out.size() << std::endl;
}

// int main() {
//     Position board = InitialBoard();
//     auto moves = Moves(1, board);
//     for (auto &move : moves) {
//         // std::cout << move.from.first << "," << move.from.second << " : " << move.to.first << "," << move.to.second << " " << piece_names[move.board.get(move.to.first, move.to.second)] << std::endl;
//         std::cout << (int) move.from.i << "," << 
//         (int) move.from.j << " : " << 
//         (int) move.to.i << "," << 
//         (int) move.to.j << " " << 
//         piece_names[move.board[8 * move.to.i + move.to.j]] << '\n';
//     }
//     return 0;
// }
