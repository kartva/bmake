"use client"

import { CMEditor } from "@/components/code_editor";
import { bgColor, borderColor, Button, containerDefault, Divider, IconButton, Input, Loading, LogoText, namePaths, Text } from "@/components/util";
import { Switch } from "@heroui/switch";
import { Tab, Tabs } from "@heroui/tabs";
import { useContext, useEffect, useState } from "react";
import { AppCtx, ModalActions, ModalCtx } from "@/components/wrapper";
import Image from "next/image";
import { IconCirclePlusFilled, IconSquareXFilled, IconX } from "@tabler/icons-react";
import { Alert, simp } from "@/components/clientutil";
import ChessBoard from "@/components/board";
import { Game } from "../../../shared";

function IconPicker({chooseIcon}: {chooseIcon: (x:string)=>void}) {
	const [search, setSearch] = useState("");
	const ctx = useContext(ModalCtx);
	return <>
		<Text v="bold" >Search for an icon</Text>
		<Input value={search} onChange={ev=>setSearch(ev.target.value)} ></Input>
		<div className={`${bgColor.secondary} mt-5 flex flex-row flex-wrap gap-2 max-h-[50dvh] overflow-y-auto`} >
			{Object.entries(namePaths).filter(([k,v])=>(
				simp(k).includes(simp(search))
			)).map(([k,v])=>
				<Button className="flex flex-col gap-1 items-center" key={k}
					onClick={()=>{
						chooseIcon(k);
						ctx?.closeModal();
					}} >
					<Image alt={k} src={v} width={50} height={50} ></Image>
					{k}
				</Button>
			)}
		</div>
	</>;
}

function TrainingModal({game}: {game: Game}) {
	const ctx = useContext(AppCtx);
	const [done, setDone] = useState<null|string>(null);

	useEffect(()=>ctx.launch(async (d)=>{
		const ws = await ctx.ws();

		d.push(ws.onMessage(msg=>{
			if (msg.type=="model_loaded") setDone(msg.id);
		}));

		ws.send({ type: "train", game });
	}), []);

	return <>
		{!done ? <Text v="md" >Give it some time (30 min)</Text>
			: <Text v="bold" >All done!</Text>}

		<ModalActions>
			{done!=null && <Button className={bgColor.sky} onClick={()=>{
				ctx.goto(`/game?id=${done}`);
			}} >
				Proceed
			</Button>}
		</ModalActions>
	</>;
}

export default function Page() {
	const ctx = useContext(AppCtx);
	const [valStatus, setValStatus] = useState<{
		type: "loading"|"ok"
	}|{
		type: "error", what: string
	}>();

	const [game, setGame] = useState<Game>(()=>{
		const s = window.localStorage.getItem("game");
		return s ? JSON.parse(s) as Game : {
			src: "-- write some Lua here", pieces: [] as string[], n: 8, m: 8, name: "",
			init: [...new Array(8)].map(_=>[...new Array(8)].map(_=>0)), autoFlip: true
		};
	});

	const [touched, setTouched] = useState(false);
	const upGame = (xd: Partial<Game>) => {
		setGame((g)=>{
			const ng = {...g};
			for (const [k,v] of Object.entries(xd)) (ng as any)[k]=v;
			window.localStorage.setItem("game", JSON.stringify(ng));
			return ng;
		});
	};

	useEffect(()=>{
		ctx.launch(async d=>{
			d.push((await ctx.ws()).onMessage((msg)=>{
				if (msg.type=="lua_validated") {
					setValStatus(msg.status=="error" ? {type: "error", what: msg.what} : {type: "ok"});
					setTouched(false);
				}
			}));
		});
	}, []);

	const tooBig = game.n*game.m > 64;
	const canContinue = !tooBig && valStatus?.type=="ok" && !touched;

	console.log(game.init);
	const goodBoard = tooBig ? [] : [...new Array(game.n)].map((_,row) =>
		[...new Array(game.m)].map((_, col) => {
			const x = row>=game.init.length || col>=game.init[row].length ? 0 : game.init[row][col];
			return x >= game.pieces.length+1 ? 0 : x;
		}));

	const [activePiece, setActivePiece] = useState(-1);

	const upGameNM = (k: "n"|"m", v: string) => {
		const h = Number.parseInt(v);
		upGame({[k]: isNaN(h) ? 5 : h});
	};

  return <>
		<div className="flex flex-row gap-2 items-center mb-10" >
			<LogoText/>
			<Divider className="h-10" />
			<Text v="big" >
				Game Editor
			</Text>
		</div>

		<Tabs>
			<Tab key="init" title="Initial Board" >
				<div className="flex flex-col gap-5" >
					<div className="max-w-[50rem] grid grid-cols-[2fr_3fr] gap-2" >
						<Text v="bold" >Name</Text>
						<Input value={game.name} pattern="^\w{5,20}$" onChange={(ev)=>{
							upGame({name: ev.target.value});
						}} />
						<Text v="bold" >Board width</Text>
						<Input type="number" min={1} step={1} value={game.m} onChange={(ev)=>{
							upGameNM("m", ev.target.value);
						}} />
						<Text v="bold" >Board height</Text>
						<Input type="number" min={1} step={1} value={game.n} onChange={(ev)=>{
							upGameNM("n", ev.target.value);
						}} />
						<Text v="bold" >Flip board by player</Text>
						<Switch isSelected={game.autoFlip} onValueChange={(ev)=>{
							upGame({autoFlip: ev});
						}} />
					</div>

					{tooBig ? <Alert bad title="Board too large" txt="Sorry about that" />
						: <div className="flex flex-row gap-3 items-start" >
							<div className="grid grid-cols-2 gap-2 auto-rows-fr" >
								{game.pieces.map((p,i)=>
									<Button className={`flex flex-col gap-1 items-center ${
											activePiece==i ? bgColor.highlight : ""
										}`} key={i}
										onClick={()=>setActivePiece(i)} >
										<Image alt="piece" src={namePaths[p]} width={50} height={50} ></Image>
									</Button>
								)}

								<Button className={`flex flex-col gap-1 items-center ${
									activePiece==-1 || activePiece>=game.pieces.length ? bgColor.highlight : ""
								}`}
									onClick={()=>setActivePiece(-1)} >
									<IconX/>
								</Button>
							</div>

							<ChessBoard board={
								{board: goodBoard, n: game.n, m: game.m, pieces: game.pieces}
							} selectSquare={c=>{
								upGame({init: game.init.toSpliced(c.i, 1, game.init[c.i].toSpliced(c.j, 1, activePiece+1))})
							}} />
						</div>}
				</div>
			</Tab>
			<Tab key="pieces" title="Pieces" >
				<div className="flex flex-row flex-wrap m-5 gap-10" >
					{game.pieces.map((p,i)=>
						<div className={`w-20 h-20 ${containerDefault} ${bgColor.highlight} p-3 flex flex-col items-center justify-center relative`} key={i} >
							<IconButton icon={<IconX/>}
								className="absolute -right-5 -top-5"
								onClick={()=>{
									upGame({pieces: game.pieces.toSpliced(i,1)})
								}} ></IconButton>
							<Image width={50} height={50} alt={p} src={namePaths[p]} ></Image>
						</div>
					)}

					<IconButton icon={<IconCirclePlusFilled/>} onClick={()=>{
						ctx.open({type: "other", name: "Choose an icon",
							modal: <IconPicker chooseIcon={icon=>upGame({
								pieces: [...game.pieces, icon]
							})} />
						});
					}} className="self-center" ></IconButton>
				</div>
			</Tab>
			<Tab key="lua" title="Lua" >
				{valStatus?.type=="error" ? <Alert bad title="Validation failed" txt={valStatus.what} className="mb-4" />
					: valStatus?.type=="loading" ? <Loading/>
					: valStatus?.type=="ok" && <div>
						<Alert title="Success" txt="Lua is good to go 👍" className="mt-4" />
						{canContinue && <Button className={`my-2 ${bgColor.sky}`} onClick={()=>{
							ctx.open({
								type: "other", name: "Training the AI",
								modal: <TrainingModal game={{
									...game, init: goodBoard
								}} />
							});
						}} >Create game</Button>}
					</div>}

				<Button
					onClick={()=>{
						ctx.launch(async ()=>{
							setValStatus({type: "loading"});
							const ws = await ctx.ws();
							ws.send({type: "submit_lua", src: game.src});
							ws.onMessage((msg)=>{
								if (msg.type=="lua_validated") {
									setValStatus(msg.status=="error" ? {type: "error", what: msg.what} : {type: "ok"});
									setTouched(false);
								}
							});
						});
					}}
					className="mb-5"
					disabled={valStatus?.type=="loading"} >
					Validate code
				</Button>

				<CMEditor source={game.src} setSource={(src)=>{
					upGame({src});
				}} />
			</Tab>
		</Tabs>
  </>;
}