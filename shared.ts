export const MaxBoardSize = 64;

export type Coordinate = {
	i: number,
	j: number
};

export type Position = {
	next_player: 0|1;
	board: number[];
}

export type Move = {
	from: Coordinate,
	to: Coordinate,
	board: number[]
};

export type MoveHistory = {
	player: 0|1,
	move: Move
}[];

export type ClientState = {
	id: string,
	game: Omit<Game,"init"|"src">,
	position: Position,
	clientPlayer: 0|1,
	moves: Move[],
	history: MoveHistory
};

export type MessageToClient = ({
	type: "lua_validated",
}&({
	status: "ok"
}|{
	status: "error", what: string
})) | {
	type: "model_loaded",
	id: string
} | (ClientState&{
	type: "board_info",
}) | {
	type: "error",
	what: string
};

export type MessageToServer = {
	type: "refresh"
} | {
	type: "train",
	game: Game
} | {
	type: "start_game",
	id: string,
	player: 0|1
} | {
	type: "move_select",
	move: Move
} | {
	type: "submit_lua",
	src: string
};

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

export type Game = {
	src: string,
	pieces: string[],
	n: number, m: number,
	init: number[][]
};
