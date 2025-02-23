import { textColor } from "@/components/util";
import { AppWrapper, GoatCounter } from "@/components/wrapper";
import { Metadata } from "next";
import { Chivo, Inter } from 'next/font/google';
import React from "react";
import banner from "../public/banner.png";
import "./style.css";

const chivo = Chivo({ subsets: ['latin'], display: 'swap', variable: "--chivo" });
const inter = Inter({ subsets: ['latin'], display: 'swap', variable: "--inter" });

const url = process.env.NEXT_PUBLIC_ROOT_URL!;
const domain = new URL(url).host;

const goatCounter = process.env.NEXT_PUBLIC_GOAT_COUNTER!==undefined && process.env.NEXT_PUBLIC_GOAT_COUNTER.length>0
  ? process.env.NEXT_PUBLIC_GOAT_COUNTER : null;

export async function generateMetadata(): Promise<Metadata> {
  return {
    title: "*fish",
    icons: "/favicon.png"
  };
}

export default async function Layout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en" className={`${inter.variable} ${chivo.variable} font-body ${textColor.default} dark:bg-neutral-950 bg-neutral-100`} >
      <head>
        <meta name='og:locality' content='West Lafayette' />
        <meta name='og:region' content='IN' />
        <meta name='og:postal-code' content='47906' />
        <meta name='og:postal-code' content='47907' />

        <meta property="twitter:domain" content={domain} />
        <meta property="twitter:url" content={url} />
      </head>
      <body>
        <AppWrapper>
          {children}
        </AppWrapper>
      </body>
    </html>
  )
}