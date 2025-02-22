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
    [1] = "P", [2] = "N", [3] = "B", [4] = "R", [5] = "Q", [6] = "K", -- White pieces
    [7] = "p", [8] = "n", [9] = "b", [10] = "r", [11] = "q", [12] = "k" -- Black pieces
}

BOARD_WIDTH = 8
BOARD_HEIGHT = 8

function Type(player, position)
    local opponent = (player == 1) and 2 or 1
    local king = {6, 12}
    local king_position = nil
    
    for i = 1, BOARD_WIDTH do
        for j = 1, BOARD_HEIGHT do
            if position.get(i, j) == king[player] then
                king_position = {i, j}
            end
        end
    end
    
    if not king_position then return "loss" end -- If king is missing, the game is lost (should not happen in real chess)
    
    local moves = Moves(player, position)
    if #moves == 0 then
        local in_check = IsInCheck(player, position)
        if in_check then
            return "loss" -- Checkmate
        else
            return "draw" -- Stalemate
        end
    end
    
    local opponent_moves = Moves(opponent, position)
    if #opponent_moves == 0 then
        local opponent_in_check = IsInCheck(opponent, position)
        if opponent_in_check then
            return "win" -- Opponent is checkmated
        else
            return "draw" -- Opponent is stalemated
        end
    end
    
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
                    table.insert(out, {
                        from = {i = i, j = j},
                        to = {i = move.to.i, j = move.to.j},
                        board = new_board
                    })
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
    
    if abs_piece == 1 then
        local dir = (piece <= 6) and -1 or 1
        local start_row = (piece <= 6) and 7 or 2
        if position.get(i + dir, j) == 0 then
            table.insert(moves, {to = {i = i + dir, j = j}})
            if i == start_row and position.get(i + 2 * dir, j) == 0 then
                table.insert(moves, {to = {i = i + 2 * dir, j = j}})
            end
        end
    end
    return moves
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