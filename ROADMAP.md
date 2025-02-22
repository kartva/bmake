# Project Roadmap: Universal Game Engine (36-Hour Hackathon)

## Phase 1: Core Engine (12 hours)
- [X] Game State Management
  - Basic board representation
  - Move validation framework
  - Lua integration for game rules
  - Position evaluation interface

- [X] Lua Rule Interface
  - Function spec: moves(player, position) -> List<Move>
  - Position type: List<(piece_type, position)>
  - Move type: List<(add|remove, piece, position)>
  - Sample implementation for chess

## Phase 2: Search & Training (12 hours)
- [ ] Search Implementation
  - Basic Minimax with alpha-beta pruning
  - Transposition table
  - Null window search
  - Move ordering optimization

- [ ] NNUE Framework
  - Basic network architecture
  - Position -> features conversion
  - Self-play training loop
  - Weight persistence

## Phase 3: Web Interface (8 hours)
- [ ] Backend
  - REST API for game state
  - WebSocket for live games
  - NNUE weight storage/retrieval

- [ ] Frontend
  - Generic board visualization
  - Move input handling
  - Game state display
  - Basic UI for game selection

## Phase 4: Integration & Polish (4 hours)
- [ ] Testing & Bug Fixes
- [ ] Performance Optimization
- [ ] Documentation
- [ ] Demo Game Implementation

## Reference Resources
- Search Algorithms: https://www.chessprogramming.org/Minimax
- NNUE Implementation: https://www.chessprogramming.org/NNUE
- Example Implementation: https://github.com/thomasahle/sunfish/blob/master/sunfish.py

## Success Criteria
1. Load and execute custom game rules in Lua
2. Train NNUE through self-play
3. Play against trained model through web interface
4. Demonstrate with at least one complete game implementation
