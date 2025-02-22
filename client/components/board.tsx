"use client";

import React, { useState, useRef, useEffect } from "react";

// Add types to match C++ structure
type Coord = {
  x: number;
  y: number;
};

type Piece = {
  pos: Coord;
  type: number;
  add: boolean;
};

type Move = {
  from: Coord;
  to: Coord;
  pieces: Piece[];
};

type ValidMovesResponse = {
  moves: Move[];
};

const initialBoard = [
  ["r", "n", "b", "q", "k", "b", "n", "r"],
  ["p", "p", "p", "p", "p", "p", "p", "p"],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["P", "P", "P", "P", "P", "P", "P", "P"],
  ["R", "N", "B", "Q", "K", "B", "N", "R"],
];

export default function ChessBoard() {
  const [selectedSquare, setSelectedSquare] = useState<[number, number] | null>(null);
  const [isLoading, setIsLoading] = useState(false);
  const [validMoves, setValidMoves] = useState<Move[]>([]);
  const abortControllerRef = useRef<AbortController | null>(null);

  useEffect(() => {
    return () => {
      // Cleanup on unmount
      abortControllerRef.current?.abort();
    };
  }, []);

  const fetchValidMoves = async (x: number, y: number) => {
    // Cancel any ongoing request
    if (abortControllerRef.current) {
      abortControllerRef.current.abort();
    }

    // Create new AbortController for this request
    abortControllerRef.current = new AbortController();
    setValidMoves([]); // Clear previous valid moves

    try {
      setIsLoading(true);
      const response = await fetch(
        `/play/valid_moves?x=${x}&y=${y}`,
        { signal: abortControllerRef.current.signal }
      );
      if (!response.ok) {
        throw new Error('Failed to fetch valid moves');
      }
      const data: ValidMovesResponse = await response.json();
      setValidMoves(data.moves || []);
    } catch (error) {
      if (error.name !== 'AbortError') {
        console.error('Error fetching valid moves:', error);
      }
    } finally {
      if (abortControllerRef.current?.signal.aborted === false) {
        setIsLoading(false);
      }
    }
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
        {initialBoard.map((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isAlternateSquare = (rowIndex + colIndex) % 2 === 0;
            const isSelected = selectedSquare?.[0] === rowIndex && selectedSquare?.[1] === colIndex;
            const isValidMoveTarget = validMoves.some(move => move.to.x === colIndex && move.to.y === rowIndex);

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
