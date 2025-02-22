"use client";

import React, { useState } from "react";
import { CMEditor } from "../components/code_editor";
import LoadingScreen from "../components/loadingscreen";
import ChessBoard from "../components/board";

export default function Home() {
  const [stage, setStage] = useState<"editor" | "loading" | "chessboard">("editor");

  const handleEditorSubmit = () => {
    setStage("loading");
    setTimeout(() => {
      setStage("chessboard");
    }, 2000); // Simulate loading time
  };

  return <>
    {stage === "editor" && <CMEditor source="-- write some Lua here" setSource={ (s) => {} } onSubmit={handleEditorSubmit} />}
    {stage === "loading" && <LoadingScreen />}
    {stage === "chessboard" && <ChessBoard />}
  </>;
}
