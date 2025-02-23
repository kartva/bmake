# *fish
<p align="center">
  <i>pronounced "starfish"</i><br>
  <b>Training AI to beat you at board games that you make.</b>
</p>

## Inspiration
Our team of competitive programmers and chess enthusiasts naturally graviated to chess engines. We enjoyed the idea of applying chess engine techniques to create a general playing framework.

## What it does
We take a Lua specification of a board game and pit our AI against you in match.

## How we built it
*Fish is composed of:
- the front end, written in Next.js and running on node.js
- the back end, written in the Hono framework running on Deno (yes, this means our websites runs TWO server processes)
- an alpha beta move search engine with quiescence, null window searches, killer move heuristic, MTD F with binary search for NegaC* written in C++.
- a NNUE (Efficiently Updatable Neural Network) to learn game weights through self-play to produce an evaluation used by the alpha-beta engine.
- the Lua code loaded by the C++ engine used for training and game playing.

## Challenges we ran into
Many. A list of our greatest hits:
- The above architecture description implies that a move made by the user goes through: Frontend (React) -> Backend (Deno) -> Engine (C++) -> Game Specification (Lua). Each of these layers uses different technology to communicate: WebSockets, Unix pipes, and the Lua FFI bindings. Pain.
- It took two rather smart people ~5 hours to specify the rules of chess in Lua (they blame it on not knowing Lua)
- The alpha-beta move search engine was initially multi-threaded. Instead of 1 problem to fix, now we had N problems! (N = number of CPU cores). Implementing the optimizations was difficult as well.
- Integrating the NNUE was difficult (and we ultimately gave up on it)

## Accomplishments that we're proud of
- We have an elegant formulation for user-defined board games expressed in Lua.
- A pretty frontend.
- The optimized move search engine.

## What we learned
We learned a lot about how chess engines work: optimizations that help prune the search space and other techniques such as using neural networks and self-play to figure out piece weights. Most of us used Lua for the first time.

## What's next for *fish
We haven't been able to finish some of the features -- in particular, integrating the NNUE into the move search engine which tunes the search heuristic to be game-specific. We've currently put in chess weights.