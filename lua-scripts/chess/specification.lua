--[[
Type Annotations:
function type(player: number, position: table)
    - player: 1 for white, 2 for black.
    - position: table array, each element:
        { type: number, c = { x: number, y: number } }
    - returns: a string "win", "loss", "draw", or nil if not an ending state

function moves(player: number, position: table): table
    - player: 1 for white, 2 for black.
    - position: table array, each element:
        { type: number, c = { x: number, y: number } }
    - returns: table array of moves.
        Each move = {
            from = {i:number, j:number},
            to   = {i:number, j:number},
            pieces = { { c = {i:number, j:number}, type: number, being_added: boolean }, ... }
        }
--]]

function type(player, position)
    found = [false, false]
    for i,p in ipairs(position) do
        found[position[i].type]=true
    end

    local other = player == 1 and 2 or 1
    if found[player] and not found[other] do
        return "win"
    else if found[other] and not found[player] do
        return "loss"
    else if not found[player] and not found[other] do
        return "draw"
    else
        return nil
end

function moves(player, position)
    local out = []
    for i,v in ipairs(position) do
        out
    end
end