import { Context, Hono } from 'hono';
import { upgradeWebSocket } from "jsr:@hono/hono/deno";
import { MessageToClient, MessageToServer, Coordinate, Move, ServerResponse, Game, Position, MoveHistory } from "../shared.ts";
import { WSContext } from "jsr:@hono/hono/ws";
import { coreIO, MainProc } from "./core_io.ts";

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

type ServerState = {
  type: "uninit"
} | {
  type: "playing",
  proc: MainProc,
  id: string,
  game: Omit<Game,"init"|"src">,
  player: 0|1,
  position: Position,
  history: MoveHistory
};

type HandleCtx = Readonly<{
  dispose: (()=>void|Promise<void>)[],
  reply: (msg: MessageToClient)=>void,
  id: string,
  state: ServerState
}>;

class BadState extends Error {
  constructor() {super("bad state");}
}

type DBGame = Game&{
  //weights
};

async function handle(msg: MessageToServer, ctx: HandleCtx): Promise<ServerState> {
  const reset = async ()=>{
    while (ctx.dispose.length>0) await ctx.dispose.pop()!();
  };

  const start = async (src: string, ...args: string[]) => {
    const luaFilePath = await Deno.makeTempFile();
    ctx.dispose.push(()=>Deno.remove(luaFilePath));
    await Deno.writeTextFile(luaFilePath, src);

    const process = new MainProc(...args, luaFilePath);
    ctx.dispose.push(()=>process.dispose());
    return process;
  };

  const refresh = async (state: Extract<(typeof ctx)["state"], {type: "playing"}>) => {
    await state.proc.send([
      state.position.next_player,
      state.game.n, state.game.m,
      ...state.position.board
    ].join(" "));

    const io = coreIO(await state.proc.readInts());
    const moves = [...new Array(io.buf.pop())].map(_=>io.receive_move());

    ctx.reply({
      type: "board_info",
      id: state.id,
      position: state.position,
      clientPlayer: state.player,
      history: state.history,
      game: state.game,
      moves
    });
  };

  switch (msg.type) {
    case "submit_lua": {
      const process = await start(msg.src, "validate");

      if ((await process.wait()).code==1) {
        ctx.reply({type: "lua_validated", status: "error", what: await process.readStdout()});
      } else {
        ctx.reply({type: "lua_validated", status: "ok"});
      }

      return ctx.state;
    }

    case "train": {
      await reset();

      const proc = await start(msg.game.src, "train");
      // ???

      const id = crypto.randomUUID();
      await kv.set([id], {
        ...msg.game
      } satisfies DBGame);

      ctx.reply({type: "model_loaded", id});

      return {type: "uninit"};
    }

    case "start_game": {
      await reset();

      const g_kv = await kv.get([msg.id]);
      if (!g_kv) throw new Error("not found");
      const g = g_kv.value as DBGame;

      const proc = await start(g.src, "play");
      await proc.send(`${g.n} ${g.m}`)

      const playState = {
        type: "playing" as const,
        proc,
        id: msg.id,
        player: msg.player as 0|1,
        game: g,
        position: {
          next_player: 0 as const,
          board: g.init.flat()
        },
        history: []
      };

      await refresh(playState);
      return playState;
    }

    case "refresh": {
      if (ctx.state.type!="playing") throw new BadState();
      await refresh(ctx.state);
      return ctx.state;
    }

    case "move_select": {
      if (ctx.state.type!="playing") throw new BadState();

      const newState = {
        ...ctx.state,
        position: {
          next_player: (ctx.state.position.next_player==1 ? 0 : 1) as 0|1,
          board: msg.move.board
        },
        history: [...ctx.state.history, {
          player: ctx.state.position.next_player,
          move: msg.move
        }]
      };

      refresh(newState);
      return newState;
    }

    //   const process = processes.get(ctx.id);
    //   if (!process) throw new Error("No active game");
    //   await process.makeMove(move);

    //   const serverMove = await process.readServerMove();

    //   ctx.reply({
    //     type: "server_move_select",
    //     move: serverMove
    //   });

    //   break;
    // }

    default: throw new Error("unsupported");
  }
}

app.get('/play', upgradeWebSocket(_c => {
	const send = (ws: WSContext, m: MessageToClient) => {
    ws.send(JSON.stringify(m));
  };

	const id = crypto.randomUUID();
  const dispose: (()=>Promise<void>|void)[] = [];
  let state: ServerState = {type: "uninit"}

	return {
		async onMessage(event, ws) {
			const msg = JSON.parse(event.data.toString()) as MessageToServer;
			try {
				state = await handle(msg, {
					dispose,
					reply: (msg)=>send(ws, msg),
					id, state
				});
			} catch (e) {
				send(ws, {type: "error", what: `${e}`});
			}
		},
    async onClose() {
      for (const x of dispose) {
        await x();
      }
    }
	};
}));

const portEnv = Deno.env.get("PORT");
const port = portEnv ? Number.parseInt(portEnv) : 5555;
console.log(`listening on port ${port}`);
Deno.serve({port}, app.fetch);