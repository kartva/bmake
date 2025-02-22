"use client";

import React, { useState, useRef, useEffect, useMemo } from "react";
import { Coordinate, Move, Piece, MessageToClient } from "../../shared.ts";
import Image from "next/image";
import { namePaths } from "./util.tsx";

type Board = {
	n: number, m: number,
	board: number[][],
	moves?: Move[],
	pieces: string[]
};

export default function ChessBoard({
	board, selectSquare, selectMove, selectedSquare1
}: {
	board: Board,
	selectedSquare1?: Coordinate,
	selectSquare?: (x: Coordinate)=>void,
	selectMove?: (x: Move["board"])=>void
}) {
	const [selectedSquare2, setSelectedSquare] = useState<Coordinate | null>(null);
	const selectedSquare = selectedSquare1 ?? selectedSquare2;
	const movesTo = useMemo(()=>
		new Map(!selectedSquare ? [] : (board.moves ?? [])
			.filter(x=>x.from.x==selectedSquare.x && x.from.y==selectedSquare.y)
			.map(v=>[`${v.to.x}-${v.to.y}`, v.board])), [selectedSquare]);

	return <div className="grid gap-[1px] bg-gray-600 p-[1px]"
		style={{
			gridTemplateColumns: `repeat(${board.m}, minmax(0, 1fr))`,
			gridTemplateRows: `repeat(${board.n}, minmax(0, 1fr))`
		}} >
		{board.board.map((vs,row) =>
			vs.map((x, col) => {
				const isSel = selectedSquare?.x == col && selectedSquare?.y == row;
				const targetMove = movesTo.get(`${col}-${row}`);

				return (
					<div
						key={`${row}-${col}`}
						className={`w-16 h-16 flex items-center justify-center relative
							${(row+col)%2 ? "bg-[#B58863]" : "bg-[#F0D9B5]"}
							${isSel ? "ring-2 ring-yellow-400 ring-inset" : ""}
							${targetMove!=undefined ? "after:absolute after:inset-0 after:bg-yellow-400 after:opacity-30" : ""}
							hover:ring-2 hover:ring-yellow-400 hover:ring-inset`}
						onClick={() => {
							if (targetMove!=undefined) {
								selectMove?.(targetMove);
								setSelectedSquare(null);
							} else if (selectSquare) {
								selectSquare({x: col, y: row});
							} else {
								setSelectedSquare({x: col, y: row});
							}
						}} >

						{x>0 && <Image width={50} height={50} src={namePaths[board.pieces[x-1]]} alt="square" ></Image>}
					</div>
				);
			})
		)}
	</div>
}
