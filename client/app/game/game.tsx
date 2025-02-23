"use client"

import { AppCtx } from "@/components/wrapper";
import { use, useContext, useEffect, useState } from "react";
import { ClientState } from "../../../shared";
import { Button, Divider, Loading, LogoText, Text } from "@/components/util";
import ChessBoard from "@/components/board";
import { IconChessKing, IconChessKingFilled, IconDeviceGamepad } from "@tabler/icons-react";
import { useTimeUntil } from "@/components/clientutil";
import Image from "next/image";
import { twMerge } from "tailwind-merge";

export const Hdr = ({str, className}: {str?: string, className?: string}) =>
	<div className={twMerge("flex flex-row gap-2 items-center mb-10", className)} >
		<LogoText/>
		{str?.trim().length ? <>
			<Divider className="h-10" />
			<Text v="big" >
				{str}
			</Text>
		</> : ""}
	</div>;

export function ClientGame({
	game, position, moves, history,
	id, clientPlayer, started, ai_loading, end
}: ClientState) {
	const ctx = useContext(AppCtx);
	const elapsed = useTimeUntil(started, true);

	const youWon = (end=="win" || end=="loss") && ((end=="loss") == (clientPlayer!=position.next_player));

	useEffect(()=>{
		if (end!=null && end!="draw" && !youWon)
			ctx.goto("/lose-video.mp4");
	}, [end, youWon]);

	return <>
		<div className="flex flex-row justify-between self-center items-center my-10 mt-0 w-full" >
			<div className="flex flex-col items-start" >
				<Hdr str={game.name} />
				<Text v="sm" >ID {id}</Text>
			</div>
			<div className="flex flex-col items-end self-start gap-2" >
				{elapsed ? <Text v="sm" >{elapsed>=60 && `${Math.floor(elapsed/60)} minutes, `}{
					`${elapsed%60} seconds`
				}</Text> : ""}

				{ai_loading && <Image src="/hmm.gif" alt="Thinking..." height={50} width={50} />}
			</div>
		</div>

		{end!=null && (
			end=="draw" ? <Text className="m-10 self-center" v="big" >It's a draw! 🤝</Text>
			: youWon ? <Text className="m-10 self-center" v="big" >You won! 👑</Text>
			: <Text className="m-10 self-center" v="big" >You suck! 😭</Text>
		)}
		
		<div className="flex flex-row gap-2 justify-evenly" >
			<div className="flex flex-col" >
				<Text v="bold" className="m-2" >
					{position.next_player!=clientPlayer ? "Opponent's Turn" : "Opponent"}
				</Text>
				<ChessBoard board={{
						n: game.n, m: game.m,
						board: [...new Array(game.n)]
							.map((_,i) => position.board.slice(i*game.m, (i+1)*game.m)),
						moves, pieces: game.pieces,
						lastMove: history.length>0 ? history[history.length-1].move : undefined
					}}
					flip={game.autoFlip == (clientPlayer==0)}
					selectMove={ai_loading ? undefined : (move) => {
						ctx.launch(async ()=>{
							(await ctx.ws()).send({type: "move_select", move});
						})
					}}
				/>
				<Text v="bold" className="m-2 self-end" >
					{position.next_player==clientPlayer ? "Your Turn" : "You"}
				</Text>
			</div>
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
	else if (game==null) return <div className="flex flex-col items-center gap-3 my-28" >
		<Text v="big" className="text-9xl" >Join game</Text>
		<Text v="md" >Choose your champion against the machines</Text>
		<div className="flex flex-row gap-2 items-center justify-center" >
			<Button icon={<IconChessKingFilled/>} onClick={()=>setPlayer(false)} >First player</Button>
			<Button icon={<IconChessKing/>} onClick={()=>setPlayer(true)} >Second player</Button>
		</div>

		<Divider/>

		<Text v="md" >or join the multiplayer lobby</Text>
		<Button icon={<IconDeviceGamepad/>} >Multiplayer</Button>
	</div>;
	else return <ClientGame {...game} />;
}