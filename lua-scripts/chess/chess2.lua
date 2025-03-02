piece_names = {
    [0] = ".",
    [1] = "P", [2] = "N", [3] = "B", [4] = "R", [5] = "Q", [6] = "K",
    [7] = "p", [8] = "n", [9] = "b", [10] = "r", [11] = "q", [12] = "k"
}

BOARD_WIDTH = 8
BOARD_HEIGHT = 8

-- Castling rights for each player.
castling_rights = {
    [1] = { king_moved = false, rook_moved = {false, false} },
    [2] = { king_moved = false, rook_moved = {false, false} }
}

-------------------------------
-- Board Object
-------------------------------
Board = {}
function Board:new(o)
    o = o or { squares = {} }
    setmetatable(o, self)
    self.__index = self
    return o
end

function Board:initFromArray(arr)
    self.squares = {}
    for i = 1, BOARD_HEIGHT do
        self.squares[i] = {}
        for j = 1, BOARD_WIDTH do
            self.squares[i][j] = arr[i][j]
        end
    end
end

function belongsToPlayer(piece, player)
    if piece == 0 then return false end
    if player == 1 then
        return piece >= 1 and piece <= 6
    else
        return piece >= 7 and piece <= 12
    end
end

function isOpponent(piece, player)
    if piece == 0 then return false end
    return not belongsToPlayer(piece, player)
end

function addCastlingMoves(piece, i, j, position, moves)
    local player = (piece <= 6) and 1 or 2
    local rights = castling_rights[player] -- do we actually update castling rights?
    if rights.king_moved then return end
    local row = (player == 1) and 1 or 8
    local king_col = 5
    local rook_cols = {1, 8}
    for idx, rook_col in ipairs(rook_cols) do
        if not rights.rook_moved[idx] then
            if position.get(row, rook_col) ~= 0 then
                local clear_path = true
                local step = (rook_col > king_col) and 1 or -1
                for col = king_col + step, rook_col - step, step do
                    if position.get(row, col) ~= 0 then
                        clear_path = false
                        break
                    end
                end
                if clear_path then
                    -- Check that king's square and crossing squares are not attacked.
                    local safe = true
                    local opponent = (player == 1) and 2 or 1
                    for col = king_col, king_col + 2*step, step do
                        if IsAttacked(position, row, col, opponent) then
                            safe = false
                            break
                        end
                    end
                    if safe and not IsInCheck(player, position) then
                        table.insert(moves, {to = {row, king_col + 2*step}, castling = true})
                    end
                end
            end
        end
    end
end

-- only attack moves
function GenerateAttackingMoves(piece, i, j, position)
    return generateMovesCommon(piece, i, j, position, true)
end

function IsAttacked(position, i, j, opponent)
    for x = 1, BOARD_HEIGHT do
        for y = 1, BOARD_WIDTH do
            local piece = position.get(x, y)
            if belongsToPlayer(piece, opponent) then
                local attacking = GenerateAttackingMoves(piece, x, y, position)
                for _, move in ipairs(attacking) do
                    if move.to[1] == i and move.to[2] == j then
                        return true
                    end
                end
            end
        end
    end
    return false
end

function IsInCheck(player, position)
    local king = (player == 1) and 6 or 12
    local king_pos = nil
    for i = 1, BOARD_HEIGHT do
        for j = 1, BOARD_WIDTH do
            if position.get(i, j) == king then
                king_pos = {i, j}
                break
            end
        end
        if king_pos then break end
    end
    
    -- print("King position: ", king_pos[1], king_pos[2])
    if not king_pos then return true end -- King "captured"?

    local opponent = (player == 1) and 2 or 1
    return IsAttacked(position, king_pos[1], king_pos[2], opponent)
end

-------------------------------
-- Common Move Generator
-------------------------------
function generateMovesCommon(piece, i, j, position, attacking)
    local moves = {}
    local player = (piece <= 6) and 1 or 2
    local abs_piece = (piece <= 6) and piece or piece - 6

    -- Pawn moves.
    if abs_piece == 1 then
        local dir = (player == 1) and 1 or -1
        if attacking then
            for _, dj in ipairs({-1, 1}) do
                local x, y = i + dir, j + dj
                if x >= 1 and x <= BOARD_HEIGHT and y >= 1 and y <= BOARD_WIDTH then
                    table.insert(moves, {to = {x, y}})
                end
            end
        else
            local start_row = (player == 1) and 2 or 7
            local promo_row = (player == 1) and BOARD_HEIGHT or 1
            local x = i + dir
            if x >= 1 and x <= BOARD_HEIGHT and position.get(x, j) == 0 then
                local move = {to = {x, j}}
                if x == promo_row then move.promotion = true end
                table.insert(moves, move)
                if i == start_row and position.get(i + 2*dir, j) == 0 then
                    table.insert(moves, {to = {i + 2*dir, j}})
                end
            end
            for _, dj in ipairs({-1, 1}) do
                local x, y = i + dir, j + dj
                if x >= 1 and x <= BOARD_HEIGHT and y >= 1 and y <= BOARD_WIDTH then
                    local target = position.get(x, y)
                    if target ~= 0 and isOpponent(target, player) then
                        local move = {to = {x, y}}
                        if x == promo_row then move.promotion = true end
                        table.insert(moves, move)
                    end
                end
            end
        end
    else
        -- Directions for other pieces.
        local directions = {
            [2] = {{-2,-1}, {-1,-2}, {1,-2}, {2,-1}, {2,1}, {1,2}, {-1,2}, {-2,1}},  -- Knight
            [3] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}},   -- Bishop
            [4] = {{-1,0}, {1,0}, {0,-1}, {0,1}},       -- Rook
            [5] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1}}, -- Queen
            [6] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}, {-1,0}, {1,0}, {0,-1}, {0,1}}   -- King
        }
        local dirs = directions[abs_piece]
        if dirs then
            for _, d in ipairs(dirs) do
                local x, y = i, j
                repeat
                    x, y = x + d[1], y + d[2]
                    if x < 1 or x > BOARD_HEIGHT or y < 1 or y > BOARD_WIDTH then break end
                    local target = position.get(x, y)
                    
                    if target ~= 0 then
                        if isOpponent(target, player) then
                            table.insert(moves, {to = {x, y}})
                        end
                        break
                    end
                    table.insert(moves, {to = {x, y}})
                until (abs_piece == 2 or abs_piece == 6) -- Knights and Kings can't slide.
            end
        end
    end

    if (not attacking) and (abs_piece == 6) then
        addCastlingMoves(piece, i, j, position, moves)
    end

    if attacking then return moves end -- ummmmm.... unclear if this is ok. added by ishan to prevent infinite recursion

    -- Remove moves that lead to check.

    local iter = 1

    while iter <= #moves do
        local move = moves[iter]
        if doesMoveLeadToCheck(piece, i, j, move, position) then
            -- print("Move leads to check: ", i, j, move.to[1], move.to[2], piece_names[piece])
            table.remove(moves, iter)
        else
            iter = iter + 1
        end
    end

    return moves
end

function doesMoveLeadToCheck(piece, i, j, move, position)
    local oldPiece = position.get(move.to[1], move.to[2])
    position.set(i, j, 0)
    position.set(move.to[1], move.to[2], piece)
    local inCheck = IsInCheck((piece <= 6) and 1 or 2, position)
    position.set(i, j, piece)
    position.set(move.to[1], move.to[2], oldPiece)
    return inCheck
end

function GenerateMoves(piece, i, j, position)
    return generateMovesCommon(piece, i, j, position, false)
end

-- moves that pose a threat
function GenerateAttackingMoves(piece, i, j, position)
    return generateMovesCommon(piece, i, j, position, true)
end

function Moves(player, position)
    -- player = player == 1 and 2 or 1

    local out = {}
    for i = 1, BOARD_HEIGHT do
        for j = 1, BOARD_WIDTH do
            local piece = position.get(i, j)
            if belongsToPlayer(piece, player) then
                local pieceMoves = GenerateMoves(piece, i, j, position)
                for _, move in ipairs(pieceMoves) do
                    local new_board = position.clone()
                    new_board.set(i, j, 0)
                    if move.promotion then
                        piece = 6 * (player - 1) + 5
                    end
                    if move.castling then
                        local rook_col = (move.to[2] == 3) and 1 or 8
                        local rook = 6 * (player - 1) + 4
                        new_board.set(i, rook_col, 0)
                        new_board.set(i, (move.to[2] == 3) and 4 or 6, rook)
                    end
                    new_board.set(move.to[1], move.to[2], piece)
                    -- print("Move: ", i, j, move.to[1], move.to[2], piece_names[piece])
                    table.insert(out, {from = {i, j}, to = {move.to[1], move.to[2]}, board = new_board})
                end
            end
        end
    end

    -- print("Total moves: ", #out)

    return out
end

function Type(player, position)
    local moves = Moves(player, position)
    local type = nil
    if #moves == 0 then
        type = IsInCheck(player, position) and "loss" or "draw"
        return type
    end

    local moves_opp = Moves(player == 1 and 2 or 1, position)
    if #moves_opp == 0 then
        type = IsInCheck(player == 1 and 2 or 1, position) and "win" or "draw"
        return type
    end
    
    local piece_count = 0
    
    -- Count the number of non-empty squares (excluding kings)
    for i = 1, BOARD_WIDTH do
        for j = 1, BOARD_HEIGHT do
            local piece = position.get(i, j)
            if piece ~= 0 and piece ~= 6 and piece ~= 12 then
                piece_count = piece_count + 1
            end
        end
    end

    -- If only two kings remain, it's a draw
    if piece_count == 0 then
        return "draw"
    end

    return nil
end


-------------------------------
-- Initial Board Setup
-------------------------------
-- InitialBoard = {
--     {4, 2, 3, 5, 6, 3, 2, 4},
--     {0, 1, 1, 1, 1, 1, 1, 1},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {1, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0}
-- }
-- 
-- jeff board
-- InitialBoard = {
--     -- {4, 2, 3, 0, 6, 0, 2, 4},
--     -- {1, 1, 1, 1, 0, 1, 1, 1},
--     -- {0, 0, 0, 0, 0, 0, 0, 0},
--     -- {0, 0, 3, 0, 1, 0, 0, 0},
--     -- {0, 0, 0, 0, 7, 0, 0, 0},
--     -- {0, 0, 0, 0, 0, 0, 0, 8},
--     -- {7, 7, 7, 7, 0, 3, 7, 7},
--     -- {10, 8, 9, 11, 12, 9, 0, 10}

--     {0, 0, 0, 0, 6, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 4, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 9, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 12, 0, 0, 0}
-- }

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

-- InitialBoard = {
--     {4, 2, 3, 5, 6, 3, 2, 4},
--     {1, 1, 1, 1, 1, 1, 1, 1},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {7, 7, 7, 7, 7, 7, 7, 7},
--     {10, 8, 9, 11, 12, 9, 8, 10}
-- }

-- InitialBoard = {
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 1, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 6, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 10, 0, 0, 0, 0},
--     {0, 0, 0, 7, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0},
--     {0, 0, 0, 0, 0, 0, 0, 0}
-- }

-- implement max length of game so we don't get stuck when a checkmate is impossible
