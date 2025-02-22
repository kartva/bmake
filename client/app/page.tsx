"use client";

import React, { useState } from "react";
import { Button } from "../components/util";

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

export default function Home() {
  const [board, setBoard] = useState(initialBoard);

  return (
    <div className="flex flex-col items-center">
      <h1 className="text-4xl font-bold mb-4">Chess Board</h1>
      <div className="grid grid-cols-8 gap-1">
        {board.map((row, rowIndex) =>
          row.map((piece, colIndex) => (
            <Button
              key={`${rowIndex}-${colIndex}`}
              className="w-16 h-16"
            >
              {piece}
            </Button>
          ))
        )}
      </div>
    </div>
  );
}
