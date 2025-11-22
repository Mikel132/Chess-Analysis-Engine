# Chess Analysis Engine

A chess analysis engine written in C for the APSC 143 project at Queen's University.

## Features

- Complete chess board representation
- Move validation for all chess pieces
- Algebraic notation move parser
- Check detection
- Interactive command-line interface
- Support for standard chess rules

## Project Structure

```
.
├── board.c         # Board representation and game logic
├── board.h         # Board data structures and function declarations
├── parser.c        # Move parsing functionality
├── parser.h        # Parser function declarations
├── panic.c         # Error handling utilities
├── panic.h         # Error handling declarations
├── main.c          # Main program and user interface
├── Makefile        # Build configuration
└── README.md       # This file
```

## Building the Project

### Prerequisites
- GCC compiler (or any C99-compatible compiler)
- Make (optional, but recommended)

### Compilation

Using Make:
```bash
make
```

Or compile manually:
```bash
gcc -Wall -Wextra -std=c99 -o chess_engine main.c board.c parser.c panic.c
```

## Running the Program

After building, run the executable:
```bash
./chess_engine
```

## Usage

The chess engine provides an interactive command-line interface with the following commands:

- `move <from><to>` - Make a move using algebraic notation (e.g., `move e2e4`)
- `print` - Display the current board state
- `reset` - Reset the board to the starting position
- `check` - Check if the current player is in check
- `help` - Display help message
- `quit` - Exit the program

### Move Format

Moves are entered in algebraic notation:
- Source square followed by destination square
- Columns: a-h
- Rows: 1-8
- Example: `e2e4` moves a piece from e2 to e4

### Example Game Session

```
> move e2e4
Move successful: e2e4

> move e7e5
Move successful: e7e5

> move g1f3
Move successful: g1f3

> print
(displays current board)

> check
No, White is not in check.

> quit
Thanks for playing!
```

## Implementation Details

### Board Representation
- 8x8 character array representing the chess board
- Capital letters for white pieces (P, R, N, B, Q, K)
- Lowercase letters for black pieces (p, r, n, b, q, k)
- Space character for empty squares

### Move Validation
The engine validates moves according to standard chess rules:
- Pawns: Forward movement, diagonal captures, two-square initial move
- Knights: L-shaped movement (2+1 or 1+2 squares)
- Bishops: Diagonal movement
- Rooks: Horizontal and vertical movement
- Queens: Combined rook and bishop movement
- Kings: One square in any direction

### Check Detection
The engine can detect when a king is in check by verifying if any opponent piece can legally capture it.

## Building and Testing

To build and run:
```bash
make run
```

To clean build artifacts:
```bash
make clean
```

To rebuild from scratch:
```bash
make rebuild
```

## Future Enhancements

Potential improvements for the engine:
- Checkmate and stalemate detection
- Castling support
- En passant captures
- Pawn promotion
- Move history and undo functionality
- AI opponent with minimax algorithm
- PGN (Portable Game Notation) file support

## License

This project is created for educational purposes as part of APSC 143 at Queen's University.

## Author

Developed for APSC 143 - Introduction to Computer Programming for Engineers
