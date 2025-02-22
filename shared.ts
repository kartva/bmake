export const MaxBoardSize = 64;

export type Coordinate = {
    x: number,
    y: number
};

export type Piece = {
    type: number,
    pos: Coordinate
};

export type Position = {
    next_player: number;
    board: number[];
}

export type Move = {
    from: Coordinate,
    to: Coordinate,
    board: number[]
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
    position: Position,
    pieceNames: Record<number, string>
} | {
    type: "server_move_select",
    move: Move
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