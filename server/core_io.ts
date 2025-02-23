/**
 * This module handles the communication between the TypeScript server and the C++ game engine.
 * It includes parsers for game state data and a process manager for handling I/O operations
 * with the engine subprocess.
 */

import { MessageToClient, Move, Coordinate, Position } from "../shared.ts";
import { toText } from "@std/streams/to-text";
import { TextLineStream } from "@std/streams/text-line-stream";
import process from "node:process";

export class MainProc {
	proc: Deno.ChildProcess;
	read: ReadableStreamDefaultReader<string>; 
	write: WritableStreamDefaultWriter<string>;
	
	constructor(...args: string[]) {
		this.proc = new Deno.Command(process.env.CPP_PATH!, {
			args,
			stdin: "piped",
			stdout: "piped",
			stderr: "piped",
		}).spawn();

		this.read = this.proc.stdout.pipeThrough(new TextDecoderStream())
			.pipeThrough(new TextLineStream()).getReader();

		const encoder = new TextEncoderStream();
		this.write = encoder.writable.getWriter();
		encoder.readable.pipeTo(this.proc.stdin);
	}
	
	async readUntil(tl=5000): Promise<string> {
		const res = await Promise.race([
			new Promise(res=>setTimeout(res, tl)).then(_=>({type: "timeout"} as const)),
			this.proc.status.then(x=>({type: "exit", status: x} as const)),
			this.read.read().then(x=>({type: "line", line: x} as const))
		]);

		if (res.type=="timeout") throw new Error("timeout");
		if (res.type=="exit") {
			throw new Error(`process exited with code ${res.status.code}, stderr ${
				await this.readStderr()
			}`);
		}

		if (res.line.value==undefined) throw new Error("expected text, got eof");
		return res.line.value;
	}

	async readInts(tl=5000) {
		return (await this.readUntil(tl)).split(/\s+/).map(x=>Number.parseInt(x));
	}

	async wait(tl=5000) {
		return await Promise.race([
			this.proc.status,
			new Promise(res=>setTimeout(res, tl)).then(()=>{
				throw new Error("timeout")
			})
		]);
	}
	
	async readStdout(): Promise<string> {
		return await toText(this.proc.stdout);
	}

	async readStderr(): Promise<string> {
		return await toText(this.proc.stderr);
	}
	
	async send(line: string): Promise<void> {
		await this.write.write(line);
		await this.write.write("\n");
	}

	async dispose() {
		await this.write.close();
		this.read.releaseLock();
		await this.proc[Symbol.asyncDispose]();
	}
}

export function coreIO(nums: number[]) {
	return {
		buf: nums.toReversed(),
		receive_coord() {
			return {i: this.buf.pop()!, j: this.buf.pop()!};
		},
		receive_board() {
			const n=this.buf.pop()!, m=this.buf.pop()!;
			return this.buf.splice(this.buf.length-n*m, n*m).toReversed();
		},
		receive_move(): Move {
			return {
				from: this.receive_coord(),
				to: this.receive_coord(),
				board: this.receive_board()
			};
		},
	};
}
