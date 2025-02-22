--[[
Type Annotations:
piece_names: table
    - key: number
    - value: string

function Type(player: number, position: table)
    - player: 1 for white, 2 for black.
    - position: 2D table array, each element:
        { type: number, i: number, j: number }
    - returns: a string "win", "loss", "draw", or nil if not an ending state

function Moves(player: number, position: table): table
    - player: 1 for white, 2 for black.
    - position: 2D table array, each element containing an integer representing the current piece
    - returns: table array of moves.
        Each move = {
            from = {i:number, j:number},
            to   = {i:number, j:number},
            board = new board
        }
--]]

piece_names = {
    [0] = ".",
    [1] = "P", [2] = "N", [3] = "B", [4] = "R", [5] = "Q", [6] = "K",
    [7] = "p", [8] = "n", [9] = "b", [10] = "r", [11] = "q", [12] = "k"
}

BOARD_WIDTH = 8
BOARD_HEIGHT = 8

castling_rights = {
    [1] = {king_moved = false, rook_moved = {false, false}},
    [2] = {king_moved = false, rook_moved = {false, false}}
}

function Type(player, position)
    local opponent = (player == 1) and 2 or 1
    if IsInCheck(player, position) then return "loss" end
    if IsInCheck(opponent, position) then return "win" end
    
    local moves = Moves(player, position)
    if #moves == 0 then return "draw" end
    
    return nil
end

function Moves(player, position)
    local out = {}
    for i = 1, BOARD_WIDTH do
        for j = 1, BOARD_HEIGHT do
            local piece = position.get(i, j)
            if (player == 1 and piece >= 1 and piece <= 6) or (player == 2 and piece >= 7 and piece <= 12) then
                local moves = GenerateMoves(piece, i, j, position)
                for _, move in ipairs(moves) do
                    local new_board = position.clone()
                    new_board.set(i, j, 0)
                    new_board.set(move.to.i, move.to.j, piece)
                    table.insert(out, {from = {i, j}, to = move.to, board = new_board})
                end
            end
        end
    end
    return out
end

function GenerateMoves(piece, i, j, position)
    local moves = {}
    local directions = {
        [2] = {{-2,-1}, {-1,-2}, {1,-2}, {2,-1}, {2,1}, {1,2}, {-1,2}, {-2,1}},
        [3] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}},
        [4] = {{-1,0}, {1,0}, {0,-1}, {0,1}},
        [5] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1}},
        [6] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1}}
    }
    
    local abs_piece = piece > 6 and piece - 6 or piece
    local piece_dirs = directions[abs_piece]
    if piece_dirs then
        for _, dir in ipairs(piece_dirs) do
            local x, y = i, j
            repeat
                x = x + dir[1]
                y = y + dir[2]
                if x >= 1 and x <= BOARD_WIDTH and y >= 1 and y <= BOARD_HEIGHT then
                    local target = position.get(x, y)
                    if target == 0 or (piece <= 6 and target >= 7) or (piece >= 7 and target <= 6) then
                        table.insert(moves, {to = {i = x, j = y}})
                    end
                end
            until abs_piece == 2 or abs_piece == 6 or (target ~= 0 and target ~= nil)
        end
    end
    
    if abs_piece == 6 then
        AddCastlingMoves(piece, i, j, position, moves)
    end
    
    return moves
end

function AddCastlingMoves(piece, i, j, position, moves)
    local player = (piece <= 6) and 1 or 2
    local rights = castling_rights[player]
    
    if rights.king_moved then return end
    
    local row = (player == 1) and 8 or 1
    local king_col = 5
    local rooks = {1, 8}
    
    for idx, rook_col in ipairs(rooks) do
        if not rights.rook_moved[idx] then
            local clear_path = true
            local step = (rook_col > king_col) and 1 or -1
            for col = king_col + step, rook_col - step, step do
                if position.get(row, col) ~= 0 then
                    clear_path = false
                    break
                end
            end
            if clear_path and not IsInCheck(player, position) then
                table.insert(moves, {to = {i = row, j = king_col + 2 * step}})
            end
        end
    end
end

function IsInCheck(player, position)
    local king = (player == 1) and 6 or 12
    local king_pos = nil
    for i = 1, BOARD_WIDTH do
        for j = 1, BOARD_HEIGHT do
            if position.get(i, j) == king then
                king_pos = {i, j}
            end
        end
    end
    if not king_pos then return true end
    
    local opponent = (player == 1) and 2 or 1
    local opponent_moves = Moves(opponent, position)
    for _, move in ipairs(opponent_moves) do
        if move.to.i == king_pos[1] and move.to.j == king_pos[2] then
            return true
        end
    end
    return false
end

InitialBoard = {
    {4, 2, 3, 5, 6, 3, 2, 4},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {7, 7, 7, 7, 7, 7, 7, 7},
    {10, 8, 9, 11, 12, 9, 8, 10}
}