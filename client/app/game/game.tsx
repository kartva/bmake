"use client"

import { AppCtx } from "@/components/wrapper";
import { use, useContext, useEffect, useState } from "react";
import { ClientState } from "../../../shared";
import { Button, Loading, LogoText, Text } from "@/components/util";
import ChessBoard from "@/components/board";
import { IconChessKing, IconChessKingFilled } from "@tabler/icons-react";

export function ClientGame({game, position, moves, history, id}: ClientState) {
	const ctx = useContext(AppCtx);

	return <>
		<div className="flex flex-col items-start max-w-96 my-10" >
			<LogoText/>
			<Text v="sm" >ID {id}</Text>
		</div>
		
		<div className="flex flex-row gap-2 justify-evenly" >
			<ChessBoard board={{
					n: game.n, m: game.m,
					board: [...new Array(game.n)]
						.map((_,i) => position.board.slice(i*game.m, (i+1)*game.m)),
					moves, pieces: game.pieces
				}}
				selectMove={(move) => {
					ctx.launch(async ()=>{
						(await ctx.ws()).send({type: "move_select", move});
					})
				}}
			/>
		</div>
	</>;
}

export function Game({params}: { params: object }) {
	const ctx = useContext(AppCtx);

	const [id, setId] = useState<string|null>(null);
	const [game, setGame] = useState<ClientState|"loading"|null>(null);

	useEffect(()=>{
		if (!("id" in params) || typeof params.id != "string") {
			ctx.handleErr("Id not in search parameters");
			ctx.goto("/");
			return;
		}

		const id = params.id;
		setId(id);

		return ctx.launch(async (d)=>{
			d.push((await ctx.ws()).onMessage((x)=>{
				if (x.type=="board_info") setGame(x);
			}));
		});
	}, []);

	const setPlayer = (snd: boolean) => {
		setGame("loading");

		ctx.launch(async ()=>{
			(await ctx.ws()).send({ type: "start_game", id: id!, player: snd ? 1 : 0 });
		});
	};

	if (id==null || game=="loading") return <Loading/>;
	else if (game==null) return <div className="flex flex-col items-center gap-6 my-14" >
		<Text v="big" >Choose your champion</Text>
		<div className="flex flex-row gap-2 items-center justify-center" >
			<Button icon={<IconChessKingFilled/>} onClick={()=>setPlayer(false)} >First player</Button>
			<Button icon={<IconChessKing/>} onClick={()=>setPlayer(true)} >Second player</Button>
		</div>
	</div>;
	else return <ClientGame {...game} />;
}