import { Context, Hono } from 'hono';
import { upgradeWebSocket } from "jsr:@hono/hono/deno";
import { MessageToClient, MessageToServer, Coordinate, Move, Piece, PieceChange } from "../shared.ts";
import { WSContext } from "jsr:@hono/hono/ws";
import { ProcessManager } from "./core_io.ts";

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
    case "query_valid_moves": {
      const { x, y } = msg;
      const process = processes.get(ctx.id);
      if (!process) throw new Error("No active game");
      
      const moves = await process.getValidMoves(x, y);
      ctx.reply({
        type: "requested_valid_moves",
        moves
      });
      break;
    }
    case "start_game": {
      const process = new ProcessManager("game.lua", "weights.dat");
      processes.set(ctx.id, process);
      ctx.dispose.push(() => {
        process.close();
        processes.delete(ctx.id);
      });
      
      const state = await process.readInitialState();
      ctx.reply(state);
      break;
    }
    case "move_select": {
      const { move } = msg;
    }
  }
}

app.get('/play', upgradeWebSocket(_c => {
	const send = (ws: WSContext, m: MessageToClient) => ws.send(JSON.stringify(m));
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