# Chess Analysis Engine - Implementation Plan

## Overview
This plan outlines the step-by-step approach to implement a chess analysis engine that parses moves, validates them, applies them to a board, and determines game outcomes.

---

## Phase 1: Data Structure Design

### 1.1 Board State Representation (`struct chess_board`)

**Fields needed:**
- `next_move_player` (already provided)
- `squares[8][8]` - 2D array representing the board
  - Each square can be: empty, or contain a piece (with color)
  - Use a struct or enum to represent piece+color combinations
- `white_can_castle_kingside` - boolean flag
- `white_can_castle_queenside` - boolean flag
- `black_can_castle_kingside` - boolean flag
- `black_can_castle_queenside` - boolean flag
- `en_passant_target` - square that can be captured en passant (or -1 if none)
  - Store as file (0-7) and rank (0-7), or as a single value

**Square representation options:**
- Option A: `enum square_content` with values like EMPTY, WHITE_PAWN, BLACK_PAWN, etc.
- Option B: Two arrays: `pieces[8][8]` (piece type) and `colors[8][8]` (player)
- Option C: Single array with struct containing piece type and color

**Recommendation:** Use Option A for simplicity and clarity.

### 1.2 Move Representation (`struct chess_move`)

**Fields needed:**
- `piece_type` (already provided)
- `from_file` - starting file (0-7, or -1 if unknown)
- `from_rank` - starting rank (0-7, or -1 if unknown)
- `to_file` - destination file (0-7)
- `to_rank` - destination rank (0-7)
- `is_capture` - boolean indicating if move captures a piece
- `is_castle_kingside` - boolean for O-O
- `is_castle_queenside` - boolean for O-O-O
- `promotion_piece` - piece type for promotion (or PIECE_PAWN if not promotion)
- `player` - which player is making the move

**Alternative:** Use a single `from_square` and `to_square` (0-63) instead of separate file/rank.

---

## Phase 2: Board Initialization

### 2.1 `board_initialize()` Implementation

**Steps:**
1. Set `next_move_player = PLAYER_WHITE`
2. Initialize all squares to EMPTY
3. Place white pieces on rank 0 (bottom):
   - Rooks at (0,0) and (7,0)
   - Knights at (1,0) and (6,0)
   - Bishops at (2,0) and (5,0)
   - Queen at (3,0)
   - King at (4,0)
   - Pawns at rank 1 (all files)
4. Place black pieces on rank 7 (top):
   - Same pattern as white
5. Set all castling flags to `true`
6. Set `en_passant_target` to indicate no en passant available

---

## Phase 3: Input Parsing

### 3.1 `parse_move()` Implementation Strategy

**High-level approach:**
1. Skip leading whitespace
2. Check for end of input (newline)
3. Determine move type:
   - **Castling:** Check for "O-O" or "O-O-O"
   - **Pawn move:** Starts with lowercase letter (file) or "x" (capture)
   - **Piece move:** Starts with uppercase letter (K, Q, R, B, N)
4. Parse accordingly
5. Skip trailing whitespace until next non-space or newline

**Detailed parsing logic:**

#### Castling (O-O or O-O-O)
- Read "O-O" or "O-O-O"
- Set appropriate castle flags
- Set piece_type to PIECE_KING

#### Pawn Moves
- Pattern: `[file]x?[file][rank][=PROMOTION]?`
- Examples: `e4`, `exd5`, `e8=Q`, `bxa1=N`
- Parse:
  1. If starts with file (a-h): normal pawn move
  2. If starts with "x": capture (need previous file)
  3. Read destination square
  4. Check for `=Q`, `=R`, `=B`, `=N` for promotion

#### Piece Moves
- Pattern: `[PIECE][disambiguation]?x?[file][rank]`
- Examples: `Nf3`, `Nxe5`, `Ndb5`, `Qd8-d6` (though this format isn't in spec)
- Parse:
  1. Read piece type (K, Q, R, B, N)
  2. Read optional disambiguation (file, rank, or both)
  3. Read optional "x" for capture
  4. Read destination square

**Error handling:**
- Invalid characters → `panicf("parse error at character '%c'", c)`
- Invalid file/rank → parse error
- Missing spaces between moves → parse error

---

## Phase 4: Move Completion

### 4.1 `board_complete_move()` Implementation

**Purpose:** Determine the starting square when it's not fully specified.

**Algorithm:**
1. If move is castling → starting square is known (king's position)
2. If `from_file` and `from_rank` are both known → move is already complete
3. Otherwise, search for pieces that could make this move:
   - Find all pieces of the correct type and color
   - For each, check if it can legally move to the destination
   - Consider:
     - Piece movement rules (knight L-shape, bishop diagonal, etc.)
     - Blocking pieces
     - Whether it's a capture (destination must have opponent piece)
     - Whether it's not a capture (destination must be empty)
     - En passant rules for pawns
4. Filter by disambiguation:
   - If `from_file` specified → only consider pieces on that file
   - If `from_rank` specified → only consider pieces on that rank
5. Count matches:
   - 0 matches → `panicf("move completion error: ...")`
   - 1 match → set `from_file` and `from_rank`
   - 2+ matches → `panicf("move completion error: ...")` (ambiguous)

**Helper functions needed:**
- `can_piece_move(piece, from, to, board)` - checks if piece can move (ignoring check)
- `find_pieces_of_type(board, piece_type, player)` - returns list of squares with that piece

---

## Phase 5: Move Application

### 5.1 `board_apply_move()` Implementation

**Steps:**
1. Validate move is complete (from_file and from_rank are set)
2. Check move legality:
   - Piece exists at starting square
   - Piece belongs to current player
   - Destination is valid
   - Move follows piece movement rules
   - If capture: destination has opponent piece
   - If not capture: destination is empty
   - Special rules:
     - Pawn: forward only, diagonal only for capture, en passant
     - Castling: all conditions met (king/rook not moved, squares empty, not in check, etc.)
3. Check if move would leave own king in check:
   - Make temporary move
   - Check if own king is attacked
   - Undo temporary move
4. If illegal → `panicf("illegal move: ...")`
5. Apply move:
   - Remove piece from starting square
   - Place piece on destination square
   - Handle captures (remove captured piece)
   - Handle castling (move rook)
   - Handle en passant (remove captured pawn)
   - Handle promotion (replace pawn with promoted piece)
6. Update board state:
   - Update castling flags (if king or rook moved)
   - Set en passant target (if pawn moved two squares)
   - Clear en passant target (if not set by this move)
   - Switch `next_move_player` (already in code)

**Helper functions needed:**
- `is_square_attacked(board, square, by_player)` - checks if square is attacked
- `is_king_in_check(board, player)` - checks if king is in check
- `apply_move_to_board(board, move)` - actually moves pieces
- `update_castling_flags(board, move)` - updates castling availability
- `update_en_passant(board, move)` - updates en passant target

---

## Phase 6: Board State Classification

### 6.1 `board_summarize()` Implementation

**Steps:**
1. Determine current player (from `next_move_player`)
2. Check if current player has any legal moves:
   - Generate all possible moves for current player
   - For each, check if it's legal (wouldn't leave king in check)
   - If any legal move exists → game incomplete
3. If no legal moves:
   - Check if current player is in check
   - If in check → checkmate (opponent wins)
   - If not in check → stalemate (draw)
4. Print appropriate message:
   - `"game incomplete"`
   - `"white wins by checkmate"`
   - `"black wins by checkmate"`
   - `"draw by stalemate"`

**Helper functions needed:**
- `generate_all_moves(board, player)` - returns list of all possible moves
- `has_legal_moves(board, player)` - checks if player has any legal moves
- `is_in_check(board, player)` - checks if player's king is in check

---

## Phase 7: Testing Strategy

### 7.1 Unit Testing Approach
- Test each function independently
- Use simple test cases from the project document
- Test edge cases:
  - Ambiguous moves
  - Invalid moves
  - Check/checkmate scenarios
  - Castling edge cases
  - En passant
  - Promotion

### 7.2 Integration Testing
- Test complete game sequences from Appendix C
- Verify output matches expected format
- Test error cases from Table 1

---

## Phase 8: Implementation Order (Recommended)

1. **Week 1: Data Structures**
   - Design and implement `struct chess_board`
   - Design and implement `struct chess_move`
   - Implement `board_initialize()`

2. **Week 2: Parsing**
   - Implement `parse_move()` for simple cases
   - Add support for all move types
   - Test with example inputs

3. **Week 3: Move Logic**
   - Implement helper functions for piece movement
   - Implement `board_complete_move()`
   - Test move completion

4. **Week 4: Move Application**
   - Implement `board_apply_move()`
   - Add check detection
   - Test move legality

5. **Week 5: Game State**
   - Implement `board_summarize()`
   - Test endgame scenarios
   - Final integration testing

6. **Week 6: Bonus (Optional)**
   - Implement move recommendation
   - Add strategic evaluation

---

## Key Implementation Details

### Coordinate System
- Files: a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7
- Ranks: 1=0 (white's starting rank), 8=7 (black's starting rank)
- Array indexing: `board[rank][file]` or `board[file][rank]` (choose one consistently)

### Piece Movement Patterns
- **Knight:** (±1, ±2) or (±2, ±1)
- **Bishop:** Diagonal (same |Δfile| = |Δrank|)
- **Rook:** Horizontal (Δrank=0) or Vertical (Δfile=0)
- **Queen:** Bishop or Rook pattern
- **King:** Adjacent squares (|Δfile| ≤ 1, |Δrank| ≤ 1)
- **Pawn:** Complex (forward, capture diagonal, en passant, promotion)

### Check Detection
- Find king's position
- Check if any opponent piece can attack that square
- Consider piece movement patterns and blocking

### Legal Move Generation
- For each piece of current player:
  - Generate all squares it could move to
  - Filter by piece movement rules
  - Filter by board state (blocking, captures)
  - Filter by check (can't leave own king in check)

---

## Common Pitfalls to Avoid

1. **Off-by-one errors** in coordinate conversion
2. **Forgetting to update castling flags** when king/rook moves
3. **Not handling en passant** correctly (only available for one move)
4. **Incorrect check detection** (must check after temporary move)
5. **Ambiguous move resolution** (must consider all pieces, not just legal ones)
6. **Memory safety** (no buffer overflows, array bounds checking)
7. **Not using only permitted functions** (getc, ungetc, printf)

---

## Next Steps

1. Start with Phase 1: Design your data structures
2. Implement `board_initialize()` to test your board representation
3. Work through parsing incrementally (start with simple pawn moves)
4. Build up move completion and application logic
5. Test frequently with the provided examples

Good luck with your implementation!

