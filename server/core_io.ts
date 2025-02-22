/**
 * This module handles the communication between the TypeScript server and the C++ game engine.
 * It includes parsers for game state data and a process manager for handling I/O operations
 * with the engine subprocess.
 */

import { MessageToClient, Piece, Move, Coordinate, PieceChange } from "../shared.ts";

class IOParser {
	private static parseCoordinate(x: string, y: string): Coordinate {
		return {
			x: parseInt(x),
			y: parseInt(y)
		};
	}
	
	private static parsePiece(type: string, x: string, y: string): Piece {
		return {
			type: parseInt(type),
			pos: this.parseCoordinate(x, y)
		};
	}

	private static parsePieceChange(type: string, x: string, y: string, being_added: string): PieceChange {
		return {
			piece: this.parsePiece(type, x, y),
			being_added: being_added === "1"
		};
	}
	
	static parseMove(lines: string[]): Move {
		let currentLine = 0;
		const [fromX, fromY, toX, toY] = lines[currentLine++].split(" ").map(Number);
		const numPieces = parseInt(lines[currentLine++]);
		const pieces: PieceChange[] = [];

		for (let i = 0; i < numPieces; i++) {
			const [type, x, y, being_added] = lines[currentLine++].split(" ");
			pieces.push(IOParser.parsePieceChange(type, x, y, being_added));
		}

		return {
			from: { x: fromX, y: fromY },
			to: { x: toX, y: toY },
			pieces
		};
	}
	
	static parseBoardInfo(lines: string[]): MessageToClient {
		const [width, height, numPieces] = lines[0].split(" ").map(Number);
		const pieces: Piece[] = [];
		let currentLine = 1;
		
		for (let i = 0; i < numPieces; i++) {
			const [type, x, y] = lines[currentLine++].split(" ");
			pieces.push(this.parsePiece(type, x, y));
		}
		
		const numPieceTypes = parseInt(lines[currentLine++]);
		const pieceNames: Record<number, string> = {};
		for (let i = 0; i < numPieceTypes; i++) {
			const [type, name] = lines[currentLine++].split(" ");
			pieceNames[parseInt(type)] = name;
		}
		
		return {
			type: "board_info",
			width,
			height,
			pieces,
			pieceNames
		};
	}
	
	static parseValidMoves(lines: string[], fromX: number, fromY: number): Move[] {
		return lines.filter(line => line.trim())
		.map(line => {
			const [toX, toY] = line.split(" ");
			return {
				from: { x: fromX, y: fromY },
				to: this.parseCoordinate(toX, toY),
				pieces: [
					{ 
						piece: {
							pos: this.parseCoordinate(toX, toY),
							type: 0
						},
						being_added: false
					}
				]
			};
		});
	}
}

export class ProcessManager {
	private process: Deno.ChildProcess;
	private decoder = new TextDecoder();
	private encoder = new TextEncoder();
	
	constructor(luaPath: string, weightsPath: string) {
		this.process = new Deno.Command("./cpp/main", {
			args: ["play", luaPath, weightsPath],
			stdin: "piped",
			stdout: "piped",
		}).spawn();
	}
	
	private async readLines(): Promise<string[]> {
		const reader = this.process.stdout.getReader();
		const { value } = await reader.read();
		const lines = this.decoder.decode(value).trim().split("\n");
		reader.releaseLock();
		return lines;
	}
	
	private async writeLine(line: string): Promise<void> {
		const writer = this.process.stdin.getWriter();
		await writer.write(this.encoder.encode(line + "\n"));
		writer.releaseLock();
	}
	
	async readInitialState(): Promise<MessageToClient> {
		const lines = await this.readLines();
		return IOParser.parseBoardInfo(lines);
	}
	
	async getValidMoves(x: number, y: number): Promise<Move[]> {
		await this.writeLine(`${x} ${y}`);
		const lines = await this.readLines();
		return IOParser.parseValidMoves(lines, x, y);
	}
	
	async makeMove(move: Move): Promise<void> {
		const moveStr = 
			`${move.from.x} ${move.from.y} ${move.to.x} ${move.to.y}\n` +
			`${move.pieces.length}\n` +
			move.pieces.map(p => 
				`${p.piece.type} ${p.piece.pos.x} ${p.piece.pos.y} ${p.being_added ? 1 : 0}`
			).join("\n");
		
		await this.writeLine(moveStr);
	}
	
	async close() {
		this.process.kill();
	}
}
