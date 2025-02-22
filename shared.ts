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

export type ServerResponse<T> = {
  status:"error",
  error: "notFound"|"unauthorized"|"badRequest"|"loading"
    |"rateLimited"|"other"|"sessionExpire"|"banned",
  message: string|null
} | {status: "ok", result: T}

export function errorName(err: (ServerResponse<unknown>&{status:"error"})["error"]) {
  let name = "Unknown error";
  switch (err) {
    case "badRequest": name = "Bad Request"; break;
    case "loading": name = "Loading"; break;
    case "notFound": name = "Not Found"; break;
    case "other": name = "Other Error"; break;
    case "rateLimited": name = "Rate Limited"; break;
    case "banned": name = "You've been banned!"; break;
    case "sessionExpire": name = "Session expired"; break;
    case "unauthorized": name = "Unauthorized"; break;
  }
  return name;
}