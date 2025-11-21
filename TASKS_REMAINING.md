# Remaining Work Overview

This file summarizes everything still required to complete the chess analysis engine,
organized by source file and aligned with the project instructions.

---

## `board.h`

- Finalize `enum square_content` so each piece/color combination and empty square is represented.
- Remove duplicated fields in `struct chess_move` and ensure every field has clear semantics.
- Document (via comments) how each struct encodes the board, move flags, and coordination system.

```c
enum square_content
{
    SQUARE_EMPTY,
    SQUARE_WHITE_PAWN, SQUARE_WHITE_KNIGHT, SQUARE_WHITE_BISHOP,
    SQUARE_WHITE_ROOK, SQUARE_WHITE_QUEEN, SQUARE_WHITE_KING,
    SQUARE_BLACK_PAWN, SQUARE_BLACK_KNIGHT, SQUARE_BLACK_BISHOP,
    SQUARE_BLACK_ROOK, SQUARE_BLACK_QUEEN, SQUARE_BLACK_KING,
};

struct chess_board
{
    enum chess_player next_move_player;
    enum square_content squares[8][8];
    int white_can_castle_kingside, white_can_castle_queenside;
    int black_can_castle_kingside, black_can_castle_queenside;
    int en_passant_file;  // -1 if none
    int en_passant_rank;  // -1 if none
};

struct chess_move
{
    enum chess_player player;
    enum chess_piece piece_type;
    int from_file;  // 0-7 or -1 if unknown
    int from_rank;  // 0-7 or -1 if unknown
    int to_file;    // 0-7
    int to_rank;    // 0-7
    int is_capture;
    int is_castle_kingside;
    int is_castle_queenside;
    int is_en_passant;
    enum chess_piece promotion_piece;  // PIECE_PAWN if no promotion
};
```

---

## `board.c`

### `board_initialize`
- Ensure all 64 squares are populated with the correct initial pieces (both colours, pawns included).
- Confirm castling flags (`white/black_can_castle_{king,queen}side`) start as true.
- Initialize `en_passant_file`/`en_passant_rank` to indicate “no target” (e.g., -1).

```c
void board_initialize(struct chess_board *board)
{
    board->next_move_player = PLAYER_WHITE;
    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 8; f++)
            board->squares[r][f] = SQUARE_EMPTY;

    // White back rank
    board->squares[0][0] = board->squares[0][7] = SQUARE_WHITE_ROOK;
    board->squares[0][1] = board->squares[0][6] = SQUARE_WHITE_KNIGHT;
    board->squares[0][2] = board->squares[0][5] = SQUARE_WHITE_BISHOP;
    board->squares[0][3] = SQUARE_WHITE_QUEEN;
    board->squares[0][4] = SQUARE_WHITE_KING;
    for (int f = 0; f < 8; f++)
        board->squares[1][f] = SQUARE_WHITE_PAWN;

    // Black back rank
    board->squares[7][0] = board->squares[7][7] = SQUARE_BLACK_ROOK;
    board->squares[7][1] = board->squares[7][6] = SQUARE_BLACK_KNIGHT;
    board->squares[7][2] = board->squares[7][5] = SQUARE_BLACK_BISHOP;
    board->squares[7][3] = SQUARE_BLACK_QUEEN;
    board->squares[7][4] = SQUARE_BLACK_KING;
    for (int f = 0; f < 8; f++)
        board->squares[6][f] = SQUARE_BLACK_PAWN;

    board->white_can_castle_kingside = board->white_can_castle_queenside = 1;
    board->black_can_castle_kingside = board->black_can_castle_queenside = 1;
    board->en_passant_file = -1;
    board->en_passant_rank = -1;
}
```

### `board_complete_move`
- Determine `move->player` from `board->next_move_player`.
- If disambiguation info is missing, search every square for pieces of the correct type and colour that *could* reach `to_file/to_rank`.
- Respect capture vs. non-capture requirements, pawn-specific rules, and any partial `from_file/from_rank`.
- Panic with “move completion error” if zero or multiple candidates remain.

```c
void board_complete_move(const struct chess_board *board, struct chess_move *move)
{
    move->player = board->next_move_player;
    if (move->is_castle_kingside || move->is_castle_queenside)
    {
        move->from_file = 4;
        move->from_rank = (move->player == PLAYER_WHITE) ? 0 : 7;
        return;
    }
    if (move->from_file >= 0 && move->from_rank >= 0)
        return;

    int candidates = 0;
    int found_file = -1, found_rank = -1;
    for (int r = 0; r < 8; r++)
    {
        for (int f = 0; f < 8; f++)
        {
            enum square_content sq = board->squares[r][f];
            if (!square_has_piece_of_color(sq, move->player))
                continue;
            if (get_piece_type(sq) != move->piece_type)
                continue;
            if (move->from_file >= 0 && move->from_file != f)
                continue;
            if (move->from_rank >= 0 && move->from_rank != r)
                continue;
            if (!can_piece_move_pattern(move->piece_type, f, r, move->to_file, move->to_rank, board, move))
                continue;
            if (!destination_matches_capture(move, board))
                continue;

            candidates++;
            found_file = f;
            found_rank = r;
        }
    }
    if (candidates != 1)
    {
        panicf("move completion error: %s %s to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->to_file,
               move->to_rank + 1);
    }
    move->from_file = found_file;
    move->from_rank = found_rank;
}
```

### `board_apply_move`
- Validate that the identified piece exists and that the destination obeys movement rules.
- Handle all special cases:
  - Pawn forward moves, double pushes, diagonal captures, promotion, and en passant.
  - Castling (including verifying squares are empty and not attacked, moving the rook, and clearing castling rights).
- Temporarily apply the move to ensure it does not leave the mover’s king in check; panic with “illegal move” otherwise.
- Update board state, captured pieces, en passant target square, and toggle `next_move_player`.

```c
void board_apply_move(struct chess_board *board, const struct chess_move *move)
{
    enum square_content *from = &board->squares[move->from_rank][move->from_file];
    enum square_content *to = &board->squares[move->to_rank][move->to_file];
    enum square_content moved_piece = *from;

    if (!square_has_piece_of_color(moved_piece, move->player))
    {
        panicf("illegal move: %s %s from %c%d to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->from_file, move->from_rank + 1,
               'a' + move->to_file, move->to_rank + 1);
    }

    if (move->is_castle_kingside || move->is_castle_queenside)
    {
        apply_castle(board, move);
    }
    else
    {
        enum square_content captured = *to;
        *to = moved_piece;
        *from = SQUARE_EMPTY;

        if (move->is_en_passant)
        {
            int cap_rank = (move->player == PLAYER_WHITE) ? move->to_rank - 1 : move->to_rank + 1;
            board->squares[cap_rank][move->to_file] = SQUARE_EMPTY;
        }

        if (move->promotion_piece != PIECE_PAWN)
        {
            *to = promotion_piece_for_player(move->promotion_piece, move->player);
        }

        if (is_in_check(board, move->player))
        {
            *from = moved_piece;
            *to = captured;
            panicf("illegal move: %s %s from %c%d to %c%d\n",
                   player_string(move->player),
                   piece_string(move->piece_type),
                   'a' + move->from_file, move->from_rank + 1,
                   'a' + move->to_file, move->to_rank + 1);
        }

        update_castling_rights(board, move);
        update_en_passant_target(board, move);
    }

    board->next_move_player = (board->next_move_player == PLAYER_WHITE) ?
        PLAYER_BLACK : PLAYER_WHITE;
}
```

### `board_summarize`
- Determine whether the player to move has any legal moves:
  - Generate every move candidate, apply legality checks (similar to `board_apply_move`), and see if at least one survives.
- Report:
  - `white wins by checkmate` or `black wins by checkmate` if no legal moves and king is in check.
  - `draw by stalemate` if no legal moves but king is not in check.
  - `game incomplete` otherwise.
- (Bonus) If game incomplete and attempting the bonus, produce the second-line suggestion output.

```c
void board_summarize(const struct chess_board *board)
{
    enum chess_player player = board->next_move_player;
    int has_move = player_has_legal_move(board, player);
    int check = is_in_check(board, player);

    if (!has_move)
    {
        if (check)
        {
            printf("%s wins by checkmate\n",
                   player_string((player == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE));
        }
        else
        {
            printf("draw by stalemate\n");
        }
    }
    else
    {
        printf("game incomplete\n");
        // Optional: suggest move here
    }
}
```

---

## `parser.c`

- Implement `parse_move` using only `getc`, `ungetc`, and `panicf` for errors.
- Support:
  - Pawn moves (`e4`, `exd5`, promotions like `e8=Q`, en passant notation).
  - Piece moves with optional disambiguation (`Nf3`, `Nxe5`, `Ndb5`).
  - Castling (`O-O`, `O-O-O`).
  - Proper separation by whitespace and newline termination.
- On syntax mistakes, emit `panicf("parse error at character '%c'\n", bad_char);`
- Return `false` when newline (`\n`/`\r`) indicates end of input; otherwise return `true` after filling the `struct chess_move`.

```c
bool parse_move(struct chess_move *move)
{
    char c;
    do
    {
        c = getc(stdin);
    } while (c == ' ');

    if (c == '\n' || c == '\r')
        return false;

    init_move(move);

    if (c == 'O')
        return parse_castle(move);
    if (c >= 'a' && c <= 'h')
        return parse_pawn_move(move, c);
    if (c == 'K' || c == 'Q' || c == 'R' || c == 'B' || c == 'N')
        return parse_piece_move(move, c);

    panicf("parse error at character '%c'\n", c);
}
```

---

## `main.c`

- No additional code is needed, but main depends on all functions above being fully implemented to run from stdin to final output without manual intervention.

---

## Documentation Deliverables

- **Proposal**: Describe your plan for board representation, move representation, and parser/completion strategy.
- **Final Report**: Explain each subsystem (board state, move parsing, completion, application, classification, and optional recommendation) and argue correctness relative to project requirements.

---

## Optional Bonus (Move Recommendation)

- Extend `board_summarize` to print a second line:
  - `suggest PLAYER PIECE from SQUARE to SQUARE`
  - or `suggest PLAYER pawn from SQUARE to SQUARE promoting to PIECE` for promotions.
- Algorithm must:
  1. Recommend forced checkmates when available within three moves.
  2. Avoid moves that allow the opponent to force mate in ≤3 moves (unless unavoidable).
  3. Provide a strategic basis for other suggestions (material, development, etc.).

---

Use this checklist to guide your implementation—once every item is completed and verified, the codebase should satisfy the APSC 143 project requirements.

