"use client";

import React, { useContext, useEffect, useRef, useState } from "react";
import { AppCtx } from "@/components/wrapper";
import { Button, LogoText, Text } from "@/components/util";
import Image from "next/image";
import { IconChessQueenFilled } from "@tabler/icons-react";

export default function Home() {
  const ctx = useContext(AppCtx);

  return <div className="flex flex-row gap-10 mx-auto items-center my-11" >
    <div className="flex flex-col gap-9 items-start" >
      <div className="flex flex-col -gap-2 items-center" >
        <LogoText/>
          <Text v="dim" className="text-lg italic" >(called starfish)</Text>
      </div>
      <Text v="md" >
        A game engine for creating chesslike board games, complete with an AI to play against.
      </Text>

      <Button icon={<IconChessQueenFilled/>} onClick={()=>{
        ctx.goto("/new");
      }} >Start building</Button>
    </div>

    <Image src="/image.png" alt="hero" width="400" height="500" />
  </div>;
}
