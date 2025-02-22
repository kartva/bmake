import { Context, Hono } from 'hono';
import { upgradeWebSocket } from "jsr:@hono/hono/deno";
import { MessageToClient, MessageToServer, Coordinate, Move, Piece } from "../shared.ts";
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

let board = new Array(64).fill(0);
const [x_dim , y_dim] = [5, 5];
// fill the first row with pawns
for (let i = 0; i < y_dim; i++) {
  board[i] = 1;
}
// fill the last row with pawns
for (let i = 0; i < y_dim; i++) {
  board[(y_dim - 1) * x_dim + i] = 2;
}

const pieceNames: Record<number, string> = {
  0: "",
  1: "pawn",
  2: "rook",
  3: "knight",
  4: "bishop",
  5: "queen",
  6: "king"
};

async function handle(msg: MessageToServer, ctx: HandleCtx) {
  switch (msg.type) {
    case "query_valid_moves": {
      const { x, y } = msg;
      // const process = processes.get(ctx.id);
      // if (!process) throw new Error("No active game");

      // const moves = await process.getValidMoves(x, y);

      const moves: Move[] = [];
      if (board[y * x_dim + x] == 1 || board[y * x_dim + x] == 2) {
        // create a copy of the board with the change
        const newBoard = board.slice();
        newBoard[(board[y * x_dim + x] == 1 ? y + 1 : y - 1) * x_dim + x] = board[y * x_dim + x];
        newBoard[y * x_dim + x] = 0;

        moves.push({
          from: { y, x },
          to: { x, y: board[y * x_dim + x] == 1 ? y + 1 : y - 1 },
          board: newBoard
        });
      }

      console.log(moves);

      ctx.reply({
        type: "requested_valid_moves",
        moves
      });
      break;
    }
    case "start_game": {
      // const process = new ProcessManager("game.lua", "weights.dat");
      // processes.set(ctx.id, process);
      // ctx.dispose.push(() => {
      //   process.close();
      //   processes.delete(ctx.id);
      // });
      
      // const state = await process.readInitialState();
      const state: MessageToClient = {
        type: "board_info",
        width: x_dim,
        height: y_dim,
        position: {
          next_player: 1,
          board
        },
        pieceNames
      };
      ctx.reply(state);
      break;
    }
    case "move_select": {
      const { move } = msg;
      // const process = processes.get(ctx.id);
      // if (!process) throw new Error("No active game");
      // await process.makeMove(move);
      
      // // Wait for and forward server's move
      // const serverMove = await process.readServerMove();

      board = move.board;

      const serverMove: Move = {
        from: { x: 1, y: 1 },
        to: { x: 1, y: 1 },
        board
      };

      setTimeout(() => ctx.reply({
        type: "server_move_select",
        move: serverMove
      }), 1000);
      break;
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

export default { fetch };