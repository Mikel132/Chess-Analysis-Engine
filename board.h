#ifndef APSC143__BOARD_H
#define APSC143__BOARD_H

// Represents one of the two players in a chess game.
enum chess_player
{
    PLAYER_WHITE,  // The player using white pieces (moves first)
    PLAYER_BLACK,  // The player using black pieces (moves second)
};

// Gets a lowercase string denoting the player.
const char *player_string(enum chess_player player);

// Represents the type of a chess piece (without color information).
// Each player has one set of these pieces:
//   - 8 pawns (p)
//   - 2 knights (N)
//   - 2 bishops (B)
//   - 2 rooks (R)
//   - 1 queen (Q)
//   - 1 king (K)
enum chess_piece
{
    PIECE_PAWN,    // Pawn piece (p) - can only move forward, captures diagonally
    PIECE_KNIGHT,  // Knight piece (N) - moves in L-shape, can jump over pieces
    PIECE_BISHOP,  // Bishop piece (B) - moves diagonally any distance
    PIECE_ROOK,    // Rook piece (R) - moves horizontally or vertically any distance
    PIECE_QUEEN,   // Queen piece (Q) - combines bishop and rook movement
    PIECE_KING,    // King piece (K) - moves one square in any direction
};
// Represents the content of a single square on the chess board.
// Each square can be empty or contain one piece of a specific color.
enum square_content
{
    SQUARE_EMPTY,              // Empty square with no piece
    SQUARE_WHITE_PAWN,         // White pawn on this square
    SQUARE_WHITE_KNIGHT,       // White knight on this square
    SQUARE_WHITE_BISHOP,       // White bishop on this square
    SQUARE_WHITE_ROOK,         // White rook on this square
    SQUARE_WHITE_QUEEN,        // White queen on this square
    SQUARE_WHITE_KING,         // White king on this square
    SQUARE_BLACK_PAWN,         // Black pawn on this square
    SQUARE_BLACK_KNIGHT,       // Black knight on this square
    SQUARE_BLACK_BISHOP,       // Black bishop on this square
    SQUARE_BLACK_ROOK,         // Black rook on this square
    SQUARE_BLACK_QUEEN,        // Black queen on this square
    SQUARE_BLACK_KING,         // Black king on this square
};

// Gets a lowercase string denoting the piece type.
const char *piece_string(enum chess_piece piece);

// Represents the complete state of a chess board and game.
// This structure contains all information needed to represent the current
// position, including piece positions, castling rights, and en passant.
struct chess_board 
{
    // Which player's turn it is to move (PLAYER_WHITE or PLAYER_BLACK).
    // White moves first in a chess game.
    enum chess_player next_move_player;
    
    // 8x8 array representing all squares on the chess board.
    // Indexing: squares[rank][file]
    //   - rank 0 = rank 1 in chess notation (white's starting rank, bottom of board)
    //   - rank 7 = rank 8 in chess notation (black's starting rank, top of board)
    //   - file 0 = file 'a' (leftmost column)
    //   - file 7 = file 'h' (rightmost column)
    // Example: squares[0][4] is square e1 (white's king starting position)
    // Example: squares[7][4] is square e8 (black's king starting position)
    enum square_content squares[8][8];
    
    // Castling rights track whether each player can still perform castling moves.
    // Once a king or rook moves, the corresponding castling right is lost.
    // 
    // White's castling rights:
    int white_can_castle_kingside;   // 1 if white can castle short (O-O), 0 otherwise
    int white_can_castle_queenside;  // 1 if white can castle long (O-O-O), 0 otherwise
    
    // Black's castling rights:
    int black_can_castle_kingside;   // 1 if black can castle short (O-O), 0 otherwise
    int black_can_castle_queenside;  // 1 if black can castle long (O-O-O), 0 otherwise
    
    // En passant target square.
    // En passant is a special pawn capture that can only occur immediately after
    // an opponent's pawn moves two squares forward from its starting position.
    // 
    // If en_passant_file is -1, there is no en passant target available.
    // Otherwise, these values indicate the square that can be captured en passant.
    // The target square is the square behind the pawn that moved two squares.
    // 
    // Example: If black pawn moves from e7 to e5, white can capture en passant
    //          by moving a pawn from d5 or f5 to e6, capturing the pawn on e5.
    //          In this case, en_passant_file = 4 (file 'e'), en_passant_rank = 4 (rank 5).
    int en_passant_file;  // File (0-7) of en passant target, or -1 if none available
    int en_passant_rank;  // Rank (0-7) of en passant target, or -1 if none available
};

// Represents a chess move, either complete or partially specified.
// When parsed from input, some fields (like from_file/from_rank) may be unknown
// and will be completed by board_complete_move().
struct chess_move
{
    // The type of piece that is moving (PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING).
    // For castling moves, this will be PIECE_KING.
    enum chess_piece piece_type;

    // Which player is making this move (PLAYER_WHITE or PLAYER_BLACK).
    // This should be set to board->next_move_player when completing the move.
    enum chess_player player;

    // Starting square coordinates (where the piece is moving from).
    // Values are 0-7 for file/rank, or -1 if not yet determined.
    // -1 means the starting square is unknown and needs to be inferred.
    // After board_complete_move() runs, these should always be valid (0-7).
    int from_file;  // Starting file: 0='a', 1='b', ..., 7='h', or -1 if unknown
    int from_rank;  // Starting rank: 0=rank1, 1=rank2, ..., 7=rank8, or -1 if unknown
    
    // Destination square coordinates (where the piece is moving to).
    // These are always known after parsing, as they are always specified in the input.
    int to_file;    // Destination file: 0='a', 1='b', ..., 7='h'
    int to_rank;    // Destination rank: 0=rank1, 1=rank2, ..., 7=rank8
    
    // Flag indicating if this move captures an opponent's piece.
    // 1 if the move captures a piece, 0 otherwise.
    // Captures are indicated by 'x' in the move notation (e.g., "exd5", "Nxe5").
    int is_capture;
    
    // Flags for special castling moves.
    // Only one of these should be 1 if it's a castling move, otherwise both are 0.
    int is_castle_kingside;   // 1 if short castle (O-O), 0 otherwise
    int is_castle_queenside;  // 1 if long castle (O-O-O), 0 otherwise
    
    // Flag indicating if this is an en passant capture.
    // 1 if the move captures en passant, 0 otherwise.
    // En passant is a special pawn capture that can occur in specific circumstances.
    int is_en_passant;
    
    // The piece type to promote to when a pawn reaches the opposite side.
    // If this is not a promotion move, this should be PIECE_PAWN.
    // Valid promotion pieces are: PIECE_QUEEN, PIECE_ROOK, PIECE_BISHOP, PIECE_KNIGHT.
    // Promotions are indicated by "=Q", "=R", "=B", or "=N" in the move notation (e.g., "e8=Q").
    enum chess_piece promotion_piece;
};

// Initializes the state of the board for a new chess game.
void board_initialize(struct chess_board *board);

// Determine which piece is moving, and complete the move data accordingly.
// Panics if there is no piece which can make the specified move, or if there
// are multiple possible pieces.
void board_complete_move(const struct chess_board *board, struct chess_move *move);

// Apply move to the board. The move must already be complete, i.e., the initial
// square must be known. Panics if the move is not legal in the current board
// position.
void board_apply_move(struct chess_board *board, const struct chess_move *move);

// Classify the state of the board, printing one of the following:
// - game incomplete
// - white wins by checkmate
// - black wins by checkmate
// - draw by stalemate
void board_summarize(const struct chess_board *board);

#endif
