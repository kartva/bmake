import { Hono } from 'hono';
import { upgradeWebSocket } from "jsr:@hono/hono/deno";
import { MessageToClient, MessageToServer, State } from "./shared.ts";
import { WSContext } from "@hono/hono/ws";

const app = new Hono()

const kv = await Deno.openKv("./db");

type HandleCtx = {
  dispose: (()=>void)[], reply: (msg: MessageToClient)=>void
};

async function handle(msg: MessageToServer, ctx: HandleCtx) {

}

app.get('/', upgradeWebSocket(async c => {
	const s = (ws: WSContext, m: MessageToClient) => ws.send(JSON.stringify(m));

	return {
		async onMessage(event, ws) {
			const msg = JSON.parse(event.data.toString()) as MessageToServer;
			const dispose: (()=>void)[] = [];
			try {
				await handle(msg, {dispose, reply: (msg)=>s(ws, msg)});
			} catch (e) {
				s(ws, {type: "error", what: `${e}`});
			} finally {
				dispose.forEach(x=>x());
			}
		}
	};
}));

const portEnv = Deno.env.get("PORT");
const port = portEnv ? Number.parseInt(portEnv) : 5555;
console.log(`listening on port ${port}`);
Deno.serve({port}, app.fetch);
