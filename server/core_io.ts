/**
 * This module handles the communication between the TypeScript server and the C++ game engine.
 * It includes parsers for game state data and a process manager for handling I/O operations
 * with the engine subprocess.
 */

import { MessageToClient, Piece, Move, Coordinate, Position } from "../shared.ts";

interface BoardDimensions {
    width: number;
    height: number;
}

class IOParser {
	static receiveCoord(line: string): Coordinate {
		const [x, y] = line.split(" ").map(Number);
		return { x, y };
	}

	static sendCoord(coord: Coordinate): string {
		return `${coord.x} ${coord.y}\n`;
	}

	static sendBoard(board: number[]): string {
		return board.join(" ") + "\n";
	}

	static receiveBoard(line: string): number[] {
		return line.split(" ").map(Number);
	}

	static sendPosition(position: Position): string {
		return `${position.next_player}\n${IOParser.sendBoard(position.board)}`;
	}

	static receivePosition(lines: string[]): Position {
		const next_player = lines[0].split(" ").map(Number)[0];
		const board = IOParser.receiveBoard(lines[1]);
		return { next_player, board };
	}

	static sendMove(move: Move): string {
		return IOParser.sendCoord(move.from) + IOParser.sendCoord(move.to) + IOParser.sendBoard(move.board);
	}

	static receiveMove(lines: string[]): Move {
		const from = IOParser.receiveCoord(lines[0]);
		const to = IOParser.receiveCoord(lines[1]);
		const board = IOParser.receiveBoard(lines[2]);
		return { from, to, board };
	}

	static parseIntroInfor(lines: string[]): MessageToClient {
		const [width, height] = lines[0].split(" ").map(Number);
		const position = IOParser.receivePosition(lines.slice(1));
		// parse a number:string array formatted as number string\n number string...
		const pieceNames: Record<number, string> = {};
		for (let i = 0; i < lines.length; i += 1) {
			const [type, name] = lines[i].split(" ");
			pieceNames[Number(type)] = name;
		}
		return { type: "board_info", width, height, position, pieceNames };
	}

	static receiveManyMoves(lines: string[]): Move[] {
		const moves: Move[] = [];
		for (let i = 0; i < lines.length; i += 3) {
			moves.push(this.receiveMove(lines.slice(i, i + 3)));
		}
		return moves;
	}
}

export class ProcessManager {
	private process: Deno.ChildProcess;
	private decoder = new TextDecoder();
	private encoder = new TextEncoder();
	
	constructor(luaPath: string, weightsPath: string) {
		this.process = new Deno.Command("../cpp/build1/main", {
			args: [luaPath, weightsPath],
			stdin: "piped",
			stdout: "piped",
			stderr: "piped",
		}).spawn();
	}
	
	private async readStdoutLines(): Promise<string[]> {
		const reader = this.process.stdout.getReader();
		const { value } = await reader.read();
		const lines = this.decoder.decode(value).trim().split("\n");
		reader.releaseLock();
		return lines;
	}

	private async readStderr(): Promise<string> {
		const reader = this.process.stderr.getReader();
		const { value } = await reader.read();
		const lines = this.decoder.decode(value).trim();
		reader.releaseLock();
		return lines;
	}
	
	private async writeLine(line: string): Promise<void> {
		const writer = this.process.stdin.getWriter();
		await writer.write(this.encoder.encode(line + "\n"));
		writer.releaseLock();
	}

	async validateLuaAndStartTraining(): Promise<MessageToClient> {
		const _ = await this.readStdoutLines(); // discard information about CLI

		await this.writeLine("validate_lua");
		const lines = await this.readStdoutLines();
		await this.writeLine("start_training");

		if (lines[0] === "validated") {
			return { type: "lua_validated", status: "ok" };
		} else {
			return { type: "lua_validated", status: "error", what: `stdout:\n${lines.join("\n")}\nstderr:\n${await this.readStderr()}` };
		}
	}

	// Wait for the engine to output a line, then trigger the callback.
	async triggerOnStdoutput(cb: ((s: string) => void)) {
		const msg = (await this.readStdoutLines()).join("\n");
		cb(msg);
	}

	async readInitialState(): Promise<MessageToClient> {
		const lines = await this.readStdoutLines();
		return IOParser.parseIntroInfor(lines);
	}
	
	async getValidMoves(x: number, y: number): Promise<Move[]> {
		console.log("getValidMoves");
		await this.writeLine(`query_valid_moves\n`);
		await this.writeLine(IOParser.sendCoord({ x, y }));

		const lines = await this.readStdoutLines();
		return IOParser.receiveManyMoves(lines);
	}
	
	async makeMove(move: Move): Promise<void> {
		const moveStr = `move_select\n` + IOParser.sendMove(move);
            
		await this.writeLine(moveStr);
	}
	
	async readServerMove(): Promise<Move> {
		const lines = await this.readStdoutLines();
		return IOParser.receiveMove(lines);
	}
	
	async close() {
		this.process.kill();
	}
}

export function getIndex(x: number, y: number, width: number): number {
    return y * width + x;
}

export function getCoord(index: number, width: number): Coordinate {
    return {
        x: index % width,
        y: Math.floor(index / width)
    };
}
