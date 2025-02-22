import { NextConfig } from "next";
import { join } from "node:path";
import { cwd } from "node:process";

export default (phase: string): NextConfig => ({
  // reactStrictMode: true,
  experimental: {
    turbo: {
      root: join(cwd(), "..")
    }
  }
});