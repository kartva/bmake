--[[
Type Annotations:
function moves(player: number, position: table): table
  - player: 1 for white, 2 for black.
  - position: table array, each element:
       { piece = { type: number }, pos = { x: number, y: number } }
  - returns: table array of moves.
       Each move = { from = {x:number, y:number},
                     to   = {x:number, y:number},
                     pieces = { { from = {x:number, y:number}, type: number, add: boolean }, ... }
                   }
--]]

-- Piece definitions
WHITE_PAWN = 1
WHITE_KNIGHT = 2
WHITE_BISHOP = 3
WHITE_ROOK = 4
WHITE_QUEEN = 5
WHITE_KING = 6
BLACK_PAWN = 7
BLACK_KNIGHT = 8
BLACK_BISHOP = 9
BLACK_ROOK = 10
BLACK_QUEEN = 11
BLACK_KING = 12

-- Piece name mapping
piece_names = {
    [WHITE_PAWN] = "wp",
    [WHITE_KNIGHT] = "wn",
    [WHITE_BISHOP] = "wb",
    [WHITE_ROOK] = "wr",
    [WHITE_QUEEN] = "wq",
    [WHITE_KING] = "wk",
    [BLACK_PAWN] = "bp",
    [BLACK_KNIGHT] = "bn",
    [BLACK_BISHOP] = "bb",
    [BLACK_ROOK] = "br",
    [BLACK_QUEEN] = "bq",
    [BLACK_KING] = "bk"
}

-- Initial board state
initial_board = {
    -- White pieces (back rank)
    {piece = {type = WHITE_ROOK}, pos = {x = 1, y = 1}},
    {piece = {type = WHITE_KNIGHT}, pos = {x = 2, y = 1}},
    {piece = {type = WHITE_BISHOP}, pos = {x = 3, y = 1}},
    {piece = {type = WHITE_QUEEN}, pos = {x = 4, y = 1}},
    {piece = {type = WHITE_KING}, pos = {x = 5, y = 1}},
    {piece = {type = WHITE_BISHOP}, pos = {x = 6, y = 1}},
    {piece = {type = WHITE_KNIGHT}, pos = {x = 7, y = 1}},
    {piece = {type = WHITE_ROOK}, pos = {x = 8, y = 1}},
    -- White pawns
    {piece = {type = WHITE_PAWN}, pos = {x = 1, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 2, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 3, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 4, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 5, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 6, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 7, y = 2}},
    {piece = {type = WHITE_PAWN}, pos = {x = 8, y = 2}},
    -- Black pieces (back rank)
    {piece = {type = BLACK_ROOK}, pos = {x = 1, y = 8}},
    {piece = {type = BLACK_KNIGHT}, pos = {x = 2, y = 8}},
    {piece = {type = BLACK_BISHOP}, pos = {x = 3, y = 8}},
    {piece = {type = BLACK_QUEEN}, pos = {x = 4, y = 8}},
    {piece = {type = BLACK_KING}, pos = {x = 5, y = 8}},
    {piece = {type = BLACK_BISHOP}, pos = {x = 6, y = 8}},
    {piece = {type = BLACK_KNIGHT}, pos = {x = 7, y = 8}},
    {piece = {type = BLACK_ROOK}, pos = {x = 8, y = 8}},
    -- Black pawns
    {piece = {type = BLACK_PAWN}, pos = {x = 1, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 2, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 3, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 4, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 5, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 6, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 7, y = 7}},
    {piece = {type = BLACK_PAWN}, pos = {x = 8, y = 7}}
}

function is_white_piece(piece_type)
    return piece_type <= WHITE_KING
end

function get_piece_color(piece_type)
    return is_white_piece(piece_type) and 1 or 2
end

-- Helper functions
function is_valid_pos(x, y)
    return x >= 1 and x <= 8 and y >= 1 and y <= 8
end

function get_piece_at(position, x, y)
    for _, piece in ipairs(position) do
        if piece.pos.x == x and piece.pos.y == y then
            return piece
        end
    end
    return nil
end

-- Main interface function
-- player: number, position: table array (each element as described above)
-- Returns table array of moves (see type annotations above)
function moves(player, position)
    local valid_moves = {}
    
    for _, piece in ipairs(position) do
        local piece_type = piece.piece.type
        local x, y = piece.pos.x, piece.pos.y
        
        if get_piece_color(piece_type) == player then
            if piece_type == WHITE_PAWN or piece_type == BLACK_PAWN then
                add_pawn_moves(valid_moves, position, x, y, piece_type)
            elseif piece_type == WHITE_KNIGHT or piece_type == BLACK_KNIGHT then
                add_knight_moves(valid_moves, position, x, y, piece_type)
            elseif piece_type == WHITE_BISHOP or piece_type == BLACK_BISHOP then
                add_bishop_moves(valid_moves, position, x, y, piece_type)
            elseif piece_type == WHITE_ROOK or piece_type == BLACK_ROOK then
                add_rook_moves(valid_moves, position, x, y, piece_type)
            elseif piece_type == WHITE_QUEEN or piece_type == BLACK_QUEEN then
                add_queen_moves(valid_moves, position, x, y, piece_type)
            elseif piece_type == WHITE_KING or piece_type == BLACK_KING then
                add_king_moves(valid_moves, position, x, y, piece_type)
            end
        end
    end
    
    return valid_moves
end

-- Move generation functions (each move is formatted as described above)

function add_pawn_moves(moves, position, x, y, piece_type)
    local direction = piece_type == WHITE_PAWN and 1 or -1
    local start_rank = piece_type == WHITE_PAWN and 2 or 7
    
    -- Forward move (single step)
    if get_piece_at(position, x, y + direction) == nil then
        moves[#moves + 1] = {
            from = {x = x, y = y},
            to = {x = x, y = y + direction},
            pieces = {
                { from = {x = x, y = y}, type = piece_type, add = false },
                { from = {x = x, y = y + direction}, type = piece_type, add = true }
            }
        }
        
        -- Double move from start
        if y == start_rank and get_piece_at(position, x, y + 2 * direction) == nil then
            moves[#moves + 1] = {
                from = {x = x, y = y},
                to = {x = x, y = y + 2 * direction},
                pieces = {
                    { from = {x = x, y = y}, type = piece_type, add = false },
                    { from = {x = x, y = y + 2 * direction}, type = piece_type, add = true }
                }
            }
        end
    end
    
    -- Captures
    for dx = -1, 1, 2 do
        local target = get_piece_at(position, x + dx, y + direction)
        if target and get_piece_color(target.piece.type) ~= get_piece_color(piece_type) then
            moves[#moves + 1] = {
                from = {x = x, y = y},
                to = {x = x + dx, y = y + direction},
                pieces = {
                    { from = {x = x, y = y}, type = piece_type, add = false },
                    { from = target.pos, type = target.piece.type, add = false },
                    { from = {x = x + dx, y = y + direction}, type = piece_type, add = true }
                }
            }
        end
    end
end

function add_knight_moves(moves, position, x, y, piece_type)
    local offsets = {{2,1}, {2,-1}, {-2,1}, {-2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2}}
    for _, offset in ipairs(offsets) do
        local nx, ny = x + offset[1], y + offset[2]
        if is_valid_pos(nx, ny) then
            local target = get_piece_at(position, nx, ny)
            if target == nil or get_piece_color(target.piece.type) ~= get_piece_color(piece_type) then
                local op = {}
                if target then
                    op[#op+1] = { from = target.pos, type = target.piece.type, add = false }
                end
                moves[#moves + 1] = {
                    from = {x = x, y = y},
                    to = {x = nx, y = ny},
                    pieces = {
                        { from = {x = x, y = y}, type = piece_type, add = false },
                        unpack(op),
                        { from = {x = nx, y = ny}, type = piece_type, add = true }
                    }
                }
            end
        end
    end
end

function add_sliding_moves(moves, position, x, y, piece_type, directions)
    for _, dir in ipairs(directions) do
        local dx, dy = dir[1], dir[2]
        local nx, ny = x + dx, y + dy
        
        while is_valid_pos(nx, ny) do
            local target = get_piece_at(position, nx, ny)
            if target == nil then
                moves[#moves + 1] = {
                    from = {x = x, y = y},
                    to = {x = nx, y = ny},
                    pieces = {
                        { from = {x = x, y = y}, type = piece_type, add = false },
                        { from = {x = nx, y = ny}, type = piece_type, add = true }
                    }
                }
            elseif get_piece_color(target.piece.type) ~= get_piece_color(piece_type) then
                moves[#moves + 1] = {
                    from = {x = x, y = y},
                    to = {x = nx, y = ny},
                    pieces = {
                        { from = {x = x, y = y}, type = piece_type, add = false },
                        { from = target.pos, type = target.piece.type, add = false },
                        { from = {x = nx, y = ny}, type = piece_type, add = true }
                    }
                }
                break
            else
                break
            end
            nx, ny = nx + dx, ny + dy
        end
    end
end

function add_bishop_moves(moves, position, x, y, piece_type)
    local directions = {{1,1}, {1,-1}, {-1,1}, {-1,-1}}
    add_sliding_moves(moves, position, x, y, piece_type, directions)
end

function add_rook_moves(moves, position, x, y, piece_type)
    local directions = {{0,1}, {0,-1}, {1,0}, {-1,0}}
    add_sliding_moves(moves, position, x, y, piece_type, directions)
end

function add_queen_moves(moves, position, x, y, piece_type)
    local directions = {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}}
    add_sliding_moves(moves, position, x, y, piece_type, directions)
end

function add_king_moves(moves, position, x, y, piece_type)
    local directions = {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}}
    for _, dir in ipairs(directions) do
        local nx, ny = x + dir[1], y + dir[2]
        if is_valid_pos(nx, ny) then
            local target = get_piece_at(position, nx, ny)
            if target == nil then
                moves[#moves + 1] = {
                    from = {x = x, y = y},
                    to = {x = nx, y = ny},
                    pieces = {
                        { from = {x = x, y = y}, type = piece_type, add = false },
                        { from = {x = nx, y = ny}, type = piece_type, add = true }
                    }
                }
            elseif get_piece_color(target.piece.type) ~= get_piece_color(piece_type) then
                moves[#moves + 1] = {
                    from = {x = x, y = y},
                    to = {x = nx, y = ny},
                    pieces = {
                        { from = {x = x, y = y}, type = piece_type, add = false },
                        { from = target.pos, type = target.piece.type, add = false },
                        { from = {x = nx, y = ny}, type = piece_type, add = true }
                    }
                }
            end
        end
    end
end
