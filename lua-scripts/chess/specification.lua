--[[
Type Annotations:
piece_names: table
    - key: number
    - value: string

function type(player: number, position: table)
    - player: 1 for white, 2 for black.
    - position: table array, each element:
        { type: number, i: number, j: number }
    - returns: a string "win", "loss", "draw", or nil if not an ending state

function moves(player: number, position: table): table
    - player: 1 for white, 2 for black.
    - position: 2d table array, each element containing an integer representing the current piece
    - returns: table array of moves.
        Each move = {
            from = {i:number, j:number},
            to   = {i:number, j:number},
            board = new board
        }
--]]

piece_names = {
    [0] = ".",
    [1] = "w",
    [2] = "b"
}

BOARD_WIDTH = 1
BOARD_HEIGHT = 10

function Type(player, position)
    local found = {false, false}
    for j = 1,BOARD_HEIGHT do
        if position.get(1,j) > 0 then
            found[position.get(1,j)] = true
        end
    end

    local other = player == 1 and 2 or 1
    if found[player] and not found[other] then
        return "win"
    elseif found[other] and not found[player] then
        return "loss"
    elseif not found[player] and not found[other] then
        return "draw"
    else
        return nil
    end
end

function Moves(player, position)
    local out = {}
    for j1=1,BOARD_HEIGHT do
        if position.get(1,j1) == player then
            for j=-1,1,1 do
                if j~=0 and j1+j >= 1 and j1+j <= BOARD_HEIGHT then
                    local new_board = position.clone()
                    new_board.set(1,j1,0)
                    new_board.set(1,j1+j,player)

                    table.insert(out, {
                        from={i=1,j=j1},
                        to={i=1,j=j+j1},
                        board=new_board
                    })
                end
            end
        end
    end

    return out
end

InitialBoard = {
    {1,0,0,0,0, 0,0,0,0,2}
}