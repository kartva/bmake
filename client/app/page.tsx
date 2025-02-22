"use client";

import React, { useContext, useEffect, useRef, useState } from "react";
import { CMEditor } from "@/components/code_editor";
import LoadingScreen from "@/components/loadingscreen";
import { AppCtx } from "@/components/wrapper";
import { errorName, ServerResponse } from "../../shared";
import { Button, Text } from "@/components/util";
import Image from "next/image";
import { IconChessQueenFilled } from "@tabler/icons-react";

type Game = {
  n: number, m: number,
  board: number[][],
};

type ValidationStatus = {
  type: "inactive"|"loading"|"ok"
} | {
  type: "error", error: string
};

type AppState = {
  type: "editor",
  valid: ValidationStatus
} | {
  type: "game"
  loading: boolean,
  activeGame?: Game
};

const EmptyBoard = Array(8).fill(null).map(() => Array(8).fill(""));

// export function ChessBoard() {
//   const ctx = useContext(AppCtx);

//   const [selectedSquare, setSelectedSquare] = useState<[number, number] | null>(null);

//   const FlatNumberTo2DStringBoard = (arr: number[], width: number) => {
//     // truncate arr to board length
//     arr = arr.slice(0, width * boardInfo!.height);
//     const string_arr = arr.map((pt: number) => pieceNames[pt]);
//     const newArr: string[][] = [];
//     for (let i = 0; i < string_arr.length; i += width) {
//       newArr.push(string_arr.slice(i, i + width));
//     }
//     console.log(newArr);
//     return newArr;
//   }

//   useEffect(() => {

//     wsRef.current.onmessage = (event) => {
//       const msg = JSON.parse(event.data) as MessageToClient;
//       switch (msg.type) {
//         case "requested_valid_moves": {
//           setValidMoves(msg.moves);
//           setIsLoading(false);
//           break;
//         }
//         case "board_info": {
//           setPieceNames(msg.pieceNames);
//           setBoardInfo(msg); // Store board info for processing after pieceNames updates
//           break;
//         }
//         case "server_move_select": {
//           boardInfo!.position.board = msg.move.board;
//           setIsServerThinking(false);
//           break;
//         }
//       }
//     };

//     wsRef.current.onopen = () => {
//       wsRef.current?.send(JSON.stringify({ type: "start_game" }));
//     };

//     return () => wsRef.current?.close();
//   }, []);

//   const fetchValidMoves = async (x: number, y: number) => {
//     setIsLoading(true);
//     setValidMoves([]); // Clear previous valid moves
//     wsRef.current?.send(JSON.stringify({
//       type: "query_valid_moves",
//       x,
//       y
//     }));
//   };

//   const handleMoveSelect = async (move: Move) => {
//     wsRef.current?.send(JSON.stringify({
//       type: "move_select",
//       move
//     }));

//     boardInfo!.position.board = move.board;
//     setSelectedSquare(null);
//     setValidMoves([]);
//     setIsServerThinking(true);
//   };

//   const handleSquareClick = async (rowIndex: number, colIndex: number) => {
//     if (selectedSquare) {
//       // Check if clicked square is a valid move target
//       const targetMove = validMoves.find(
//         move => move.to.x === colIndex && move.to.y === rowIndex
//       );
//       if (targetMove) {
//         await handleMoveSelect(targetMove);
//         return;
//       }
//     }
//     setSelectedSquare([rowIndex, colIndex]);
//     await fetchValidMoves(colIndex, rowIndex);
//   };

//   const handleContainerClick = (e: React.MouseEvent) => {
//     // Only deselect if clicking the container itself, not a square
//     if (e.target === e.currentTarget) {
//       setSelectedSquare(null);
//       setValidMoves([]); // Clear valid moves when deselecting
//     }
//   };

//   return (
//     <div className="flex flex-col items-center" onClick={handleContainerClick}>
//       <h1 className="text-4xl font-bold mb-4">Chess Board</h1>
//       {boardInfo && boardInfo.type == "board_info" && (
//         <div 
//           className="grid gap-[1px] bg-gray-600 p-[1px]"
//           style={{
//             gridTemplateColumns: `repeat(${boardInfo.width}, minmax(0, 1fr))`,
//             gridTemplateRows: `repeat(${boardInfo.height}, minmax(0, 1fr))`
//           }}
//         >
//           {FlatNumberTo2DStringBoard(boardInfo.position.board, boardInfo.width).map((row, rowIndex) =>
//             row.map((piece, colIndex) => {
//               const isAlternateSquare = (rowIndex + colIndex) % 2 === 0;
//               const isSelected = selectedSquare?.[0] === rowIndex && selectedSquare?.[1] === colIndex;
//               const isValidMoveTarget = validMoves.some(move => move.to.x === colIndex && move.to.y === rowIndex);

//               return (
//                 <div
//                   key={`${rowIndex}-${colIndex}`}
//                   className={`w-16 h-16 flex items-center justify-center relative
//                     ${isAlternateSquare ? "bg-gray-700" : "bg-gray-800"}
//                     ${isSelected ? "ring-2 ring-yellow-400 ring-inset" : ""}
//                     ${isSelected && isLoading ? "opacity-50" : ""}
//                     ${isValidMoveTarget ? "after:absolute after:inset-0 after:bg-yellow-400 after:opacity-30" : ""}
//                     hover:ring-2 hover:ring-yellow-400 hover:ring-inset`}
//                   onClick={() => handleSquareClick(rowIndex, colIndex)}
//                 >
//                   {piece}
//                 </div>
//               );
//             })
//           )}
//         </div>
//       )}
//     </div>
//   );
// }
export default function Home() {
  const ctx = useContext(AppCtx);

  return <div className="flex flex-row gap-10 mx-auto items-center my-11" >
    <div className="flex flex-col gap-2 items-start" >
      <Text v="big" className="text-9xl" >esochess</Text>
      <Text v="md" >
        A game engine for creating chesslike board games -- and an AI to play against.
      </Text>

      <Button icon={<IconChessQueenFilled/>} onClick={()=>{
        ctx.goto("/new");
      }} >Start building</Button>
    </div>

    <Image src="/image.png" alt="hero" width="400" height="500" />
  </div>;
}
