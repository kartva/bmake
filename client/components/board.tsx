"use client";

import React, { useState, useRef, useEffect } from "react";
import { Coordinate, Move, Piece, MessageToClient } from "../../shared.ts";

const EmptyBoard = Array(8).fill(null).map(() => Array(8).fill(""));

export default function ChessBoard() {
  const [selectedSquare, setSelectedSquare] = useState<[number, number] | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [validMoves, setValidMoves] = useState<Move[]>([]);
  const [board, setBoard] = useState<string[][]>(EmptyBoard);
  const [pieceNames, setPieceNames] = useState<Record<number, string>>({});
  const [boardInfo, setBoardInfo] = useState<MessageToClient | null>(null);
  const wsRef = useRef<WebSocket | null>(null);

  useEffect(() => {
    if (boardInfo != null && boardInfo.type == "board_info" && Object.keys(pieceNames).length > 0) {
      const newBoard = Array(boardInfo.height).fill(null).map(() => Array(boardInfo.width).fill(""));
      boardInfo.pieces.forEach((piece: Piece) => {
        newBoard[piece.pos.y - 1][piece.pos.x - 1] = pieceNames[piece.type];
      });
      setBoard(newBoard);
      setBoardInfo(null); // Clear stored board info
    }
  }, [pieceNames, boardInfo]);

  useEffect(() => {
    wsRef.current = new WebSocket(new URL("/play", process.env.NEXT_PUBLIC_SERVER_URL));

    wsRef.current.onmessage = (event) => {
      const msg = JSON.parse(event.data) as MessageToClient;
      switch (msg.type) {
        case "requested_valid_moves": {
          setValidMoves(msg.moves);
          setIsLoading(false);
          break;
        }
        case "board_info": {
          setPieceNames(msg.pieceNames);
          setBoardInfo(msg); // Store board info for processing after pieceNames updates
          break;
        }
      }
    };

    wsRef.current.onopen = () => {
      wsRef.current?.send(JSON.stringify({ type: "start_game" }));
    };

    return () => wsRef.current?.close();
  }, []);

  const fetchValidMoves = async (x: number, y: number) => {
    setIsLoading(true);
    setValidMoves([]); // Clear previous valid moves
    wsRef.current?.send(JSON.stringify({
      type: "query_valid_moves",
      x,
      y
    }));
  };

  const handleSquareClick = async (rowIndex: number, colIndex: number) => {
    setSelectedSquare([rowIndex, colIndex]);
    await fetchValidMoves(colIndex, rowIndex); // Note: x = colIndex, y = rowIndex
  };

  const handleContainerClick = (e: React.MouseEvent) => {
    // Only deselect if clicking the container itself, not a square
    if (e.target === e.currentTarget) {
      setSelectedSquare(null);
      setValidMoves([]); // Clear valid moves when deselecting
    }
  };

  return (
    <div className="flex flex-col items-center" onClick={handleContainerClick}>
      <h1 className="text-4xl font-bold mb-4">Chess Board</h1>
      <div className={`grid grid-cols-8 grid-rows-8 gap-[1px] bg-gray-600 p-[1px]`}>
        {board.map((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isAlternateSquare = (rowIndex + colIndex) % 2 === 0;
            const isSelected = selectedSquare?.[0] === rowIndex && selectedSquare?.[1] === colIndex;
            const isValidMoveTarget = validMoves.some(move => move.to.x === colIndex + 1 && move.to.y === rowIndex + 1);

            return (
              <div
                key={`${rowIndex}-${colIndex}`}
                className={`w-16 h-16 flex items-center justify-center relative
                  ${isAlternateSquare ? "bg-gray-700" : "bg-gray-800"}
                  ${isSelected ? "ring-2 ring-yellow-400 ring-inset" : ""}
                  ${isSelected && isLoading ? "opacity-50" : ""}
                  ${isValidMoveTarget ? "after:absolute after:inset-0 after:bg-yellow-400 after:opacity-30" : ""}
                  hover:ring-2 hover:ring-yellow-400 hover:ring-inset`}
                onClick={() => handleSquareClick(rowIndex, colIndex)}
              >
                {piece}
              </div>
            );
          })
        )}
      </div>
    </div>
  );
}
