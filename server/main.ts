import { Context, Hono } from 'hono';
import { upgradeWebSocket } from "jsr:@hono/hono/deno";
import { MessageToClient, MessageToServer, Coordinate, Move, Piece, ServerResponse } from "../shared.ts";
import { WSContext } from "jsr:@hono/hono/ws";
import { ProcessManager } from "./core_io.ts";

// WebSocket Message Flow:
// 1. Client connects via WebSocket
// 2. Client sends "submit_lua" with Lua source code
//    - Server spawns C++ process with "play" mode
//    - Server creates temp files for Lua and weights
//    - C++ process validates Lua and responds with "validated"
//    - Server tells C++ process to "start_training"
//    - C++ process loads model and responds with "loaded"
//    - Server redirects client to board page
// 3. Client sends "start_game"
//    - C++ process sends initial board state
//    - Server sends "board_info" to client
// 4. Game loop:
//    a. Client sends "query_valid_moves" with coordinates
//       - C++ process calculates valid moves for piece
//       - Server forwards moves to client
//    b. Client sends "move_select" with chosen move
//       - C++ process updates board state
//       - C++ process calculates and makes AI move
//       - Server sends "server_move_select" to client
//    c. Repeat from 4a until game ends

const app = new Hono()

const kv = await Deno.openKv("./db");

type HandleCtx = {
  dispose: (()=>void)[],
  reply: (msg: MessageToClient)=>void,
  id: string
};

const processes = new Map<string, ProcessManager>();

async function handle(msg: MessageToServer, ctx: HandleCtx) {
  switch (msg.type) {
    case "submit_lua": {
      const { src } = msg;
      // save the lua script to a temp file

      const luaFilePath = await Deno.makeTempFile();
      await Deno.writeTextFile(luaFilePath, src);
      const weightsFilePath = await Deno.makeTempFile();
      await Deno.writeTextFile(weightsFilePath, "");

      const process = new ProcessManager(luaFilePath, weightsFilePath);
      processes.set(ctx.id, process);

      if (!process) throw new Error("No active game");
      // wait for process to either output "validated" or exit with non-zero code
      const response = await process.validateLuaAndStartTraining();
      if (response.type == "lua_validated" && response.status != "ok") {
        console.warn("Lua validation failed");
        processes.delete(ctx.id);
        ctx.reply(response);
        break;
      }
        
      process.triggerOnStdoutput((_msg) => {
        ctx.reply({ type: "model_loaded" });
      });
      break;
    }
    case "start_game": {
      const process = processes.get(ctx.id);
      if (!process) {
        throw new Error("No active game");
      }

      const state = await process.readInitialState();
      ctx.reply(state);
      break;
    }
    case "query_valid_moves": {
      const { x, y } = msg;
      const process = processes.get(ctx.id);
      if (!process) throw new Error("No active game");

      const moves = await process.getValidMoves(x, y);

      console.log(moves);

      ctx.reply({
        type: "requested_valid_moves",
        moves
      });
      break;
    }
    case "move_select": {
      const { move } = msg;

      const process = processes.get(ctx.id);
      if (!process) throw new Error("No active game");
      await process.makeMove(move);

      const serverMove = await process.readServerMove();

      ctx.reply({
        type: "server_move_select",
        move: serverMove
      });
      break;
    }
  }
}

app.get('/play', upgradeWebSocket(_c => {
	const send = (ws: WSContext, m: MessageToClient) => {
    ws.send(JSON.stringify(m));
  };

	const id = crypto.randomUUID();

	return {
		async onMessage(event, ws) {
			const msg = JSON.parse(event.data.toString()) as MessageToServer;
			const dispose: (()=>void)[] = [];
			try {
				await handle(msg, {
					dispose,
					reply: (msg)=>send(ws, msg),
					id
				});
			} catch (e) {
				send(ws, {type: "error", what: `${e}`});
			} finally {
				dispose.forEach(x=>x());
			}
		},
		onClose() {
			const process = processes.get(id);
			if (process) {
				process.close();
				processes.delete(id);
			}
		}
	};
}));

const portEnv = Deno.env.get("PORT");
const port = portEnv ? Number.parseInt(portEnv) : 5555;
console.log(`listening on port ${port}`);
Deno.serve({port}, app.fetch);

export default { fetch };