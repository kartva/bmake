"use client";

import React, { useState, useRef, useEffect, useMemo } from "react";
import { Coordinate, Move, MessageToClient } from "../../shared.ts";
import Image from "next/image";
import { namePaths } from "./util.tsx";

type Board = {
	n: number, m: number,
	board: number[][],
	moves?: Move[],
	pieces: string[],
	lastMove?: Move
};

export default function ChessBoard({
	board, selectSquare, selectMove, selectedSquare1, flip
}: {
	board: Board,
	selectedSquare1?: Coordinate,
	selectSquare?: (x: Coordinate)=>void,
	selectMove?: (x: Move)=>void,
	flip?: boolean
}) {
	const [selectedSquare2, setSelectedSquare] = useState<Coordinate | null>(null);
	const selectedSquare = selectedSquare1 ?? selectedSquare2;
	const movesTo = useMemo(()=>
		new Map(!selectedSquare ? [] : (board.moves ?? [])
			.filter(x=>x.from.j==selectedSquare.j && x.from.i==selectedSquare.i)
			.map(v=>[`${v.to.j}-${v.to.i}`, v])), [selectedSquare, board.moves]);

	const container = useRef<HTMLDivElement>(null);
	useEffect(()=>{
		const listener = (ev: MouseEvent) => {
			if (ev.target instanceof HTMLElement && !container.current!.contains(ev.target)) {
				setSelectedSquare(null);
			}
		};

		document.addEventListener("pointerdown", listener);
		return ()=>document.removeEventListener("pointerdown", listener);
	}, []);

	const b2 = board.board.map((x,i)=>[x,i] as const);
	const lasts = [board.lastMove?.to, board.lastMove?.from].map(x=>x ? `${x.i}-${x.j}` : null);

	return <div className="grid gap-[1px] bg-gray-600 p-[1px]"
		style={{
			gridTemplateColumns: `repeat(${board.m}, minmax(0, 1fr))`,
			gridTemplateRows: `repeat(${board.n}, minmax(0, 1fr))`
		}} ref={container} >
		{(flip ? b2.toReversed() : b2).map(([vs,row]) =>
			vs.map((x, col) => {
				const isSel = selectedSquare?.j == col && selectedSquare?.i == row;
				const targetMove = movesTo.get(`${col}-${row}`);

				return (
					<div
						key={`${row}-${col}`}
						className={`w-20 h-20 flex items-center justify-center relative
							${(row+col)%2 ? "bg-[#B58863]" : "bg-[#F0D9B5]"}
							${isSel ? "ring-2 ring-yellow-400 ring-inset" : ""}
							${targetMove!=undefined ? "after:absolute after:inset-0 after:bg-yellow-400 after:opacity-30" : ""}
							${lasts.includes(`${row}-${col}`) ? "ring-2 ring-blue-400 ring-inset" : ""}
							hover:ring-2 hover:ring-yellow-400 hover:ring-inset`}
						onClick={() => {
							if (targetMove!=undefined && selectMove) {
								selectMove(targetMove);
								setSelectedSquare(null);
							} else if (selectSquare) {
								selectSquare({j: col, i: row});
							} else {
								setSelectedSquare({j: col, i: row});
							}
						}} >

						{x>0 && <Image width={80} height={80} src={namePaths[board.pieces[x-1]]} alt="square" className="select-none" ></Image>}
					</div>
				);
			})
		)}
	</div>
}
