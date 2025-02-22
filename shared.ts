export type Coordinate = {
    x: number,
    y: number
};

export type Piece = {
    type: number,
    pos: Coordinate
};

export type PieceChange = {
    piece: Piece,
    being_added: boolean
};

export type Move = {
    from: Coordinate,
    to: Coordinate,
    pieces: PieceChange[]
};

export type MessageToClient = {
    type: "error",
    what: string
} | {
    type: "requested_valid_moves",
    moves: Move[]
} | {
    type: "board_info",
    width: number,
    height: number,
    pieces: Piece[],
    pieceNames: Record<number, string>
};

export type MessageToServer = {
    type: "query_valid_moves",
    x: number,
    y: number
} | {
    type: "start_game"
} | {
    type: "move_select",
    move: Move
};

export type State = {};