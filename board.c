#include "board.h"
#include <stdio.h>
#include "panic.h"

const char *player_string(enum chess_player player)
{
    switch (player)
    {
    case PLAYER_WHITE:
        return "white";
    case PLAYER_BLACK:
        return "black";
    default:
        return "unknown";
    }
}

const char *piece_string(enum chess_piece piece)
{
    switch (piece)
    {
    case PIECE_PAWN:
        return "pawn";
    case PIECE_KNIGHT:
        return "knight";
    case PIECE_BISHOP:
        return "bishop";
    case PIECE_ROOK:
        return "rook";
    case PIECE_QUEEN:
        return "queen";
    case PIECE_KING:
        return "king";
    default:
        return "unknown";
    }
}
// helper function to get the absolute value of a number since we cannot use the stdlib functions. WOMP WOMP.
static int absolute_value(int x)
{
    if (x < 0)
    {
        return -x;
    }
    return x;
}
// This is to check if a a square contains a piece of a specific player.
static int is_square_contains_piece_of_player(enum square_content square, enum chess_player player)
{
    if (square == SQUARE_EMPTY)
    {
        return 0;
    }
    if (player == PLAYER_WHITE)
    {
        return square == SQUARE_WHITE_PAWN || square == SQUARE_WHITE_KNIGHT || square == SQUARE_WHITE_BISHOP || square == SQUARE_WHITE_ROOK || square == SQUARE_WHITE_QUEEN || square == SQUARE_WHITE_KING;
    }
    else
    {
        return square == SQUARE_BLACK_PAWN || square == SQUARE_BLACK_KNIGHT || square == SQUARE_BLACK_BISHOP || square == SQUARE_BLACK_ROOK || square == SQUARE_BLACK_QUEEN || square == SQUARE_BLACK_KING;
    }
}       
// This is to get piece type of a square not regarding piece color
static enum chess_piece get_piece_type_from_square(enum square_content square)
{
    // Switch statement to determine the piece type based on the square content
    switch (square)
    {
    // White or black pawn returns PIECE_PAWN
    case SQUARE_WHITE_PAWN:
    case SQUARE_BLACK_PAWN:
        return PIECE_PAWN;
    // White or black knight returns PIECE_KNIGHT
    case SQUARE_WHITE_KNIGHT:
    case SQUARE_BLACK_KNIGHT:
        return PIECE_KNIGHT;
    // White or black bishop returns PIECE_BISHOP
    case SQUARE_WHITE_BISHOP:
    case SQUARE_BLACK_BISHOP:
        return PIECE_BISHOP;
    // White or black rook returns PIECE_ROOK
    case SQUARE_WHITE_ROOK:
    case SQUARE_BLACK_ROOK:
        return PIECE_ROOK;
    // White or black queen returns PIECE_QUEEN
    case SQUARE_WHITE_QUEEN:
    case SQUARE_BLACK_QUEEN:
        return PIECE_QUEEN;
    // White or black king returns PIECE_KING
    case SQUARE_WHITE_KING:
    case SQUARE_BLACK_KING:
        return PIECE_KING;
    // Handle invalid square content by triggering a panic
    default:
        panicf("Invalid square content for getting piece type.\\n");
    }
}
// This is to get the opponent player of a specific player  
static enum chess_player get_opponent_player(enum chess_player player)
{
    if (player == PLAYER_WHITE)
    {
        return PLAYER_BLACK;
    }
    return PLAYER_WHITE;
}

//This is to check if there is a path open for a piece to move diagonally without any pieces in the way.
static int is_path_open_diagonally(const enum square_content (*board)[8], int from_file, int from_rank, int to_file, int to_rank)
{
    int file_diff = to_file - from_file;
    int rank_diff = to_rank - from_rank;
    int file_step = file_diff > 0 ? 1 : -1;
    int rank_step = rank_diff > 0 ? 1 : -1;
    int current_file = from_file;
    int current_rank = from_rank;
    //This is a loop that checks iff there is a piece in the way of the path it would need to follow.
    int i;
    for (i = 1; i < absolute_value(file_diff); i++)
    {
        current_file += file_step;
        current_rank += rank_step;
        if (board[current_rank][current_file] != SQUARE_EMPTY)
        {
            return 0;
        }
    }
    return 1;
}
// This is to check if there is a path open for a piece to move horizontally without any pieces in the way.
static int is_path_open_horizontally(const enum square_content (*board)[8], int from_file, int from_rank, int to_file, int to_rank)
{
    int file_diff = to_file - from_file;
    int file_step = file_diff > 0 ? 1 : -1;
    int current_file = from_file;
    int current_rank = from_rank;  // Rank stays constant for horizontal moves
    //This is a loop that checks iff there is a piece in the way of the path it would need to follow.
    int i;
    for (i = 1; i < absolute_value(file_diff); i++)
    {
        current_file += file_step;
        // Rank does not change for horizontal moves
        if (board[current_rank][current_file] != SQUARE_EMPTY)
        {
            return 0;
        }
    }
    return 1;
}
// This is to check if there is a path open for a piece to move vertically without any pieces in the way.
static int is_path_open_vertically(const enum square_content (*board)[8], int from_file, int from_rank, int to_file, int to_rank)
{
    int rank_diff = to_rank - from_rank;
    int rank_step = rank_diff > 0 ? 1 : -1;
    int current_file = from_file;  // File stays constant for vertical moves
    int current_rank = from_rank;
    //This is a loop that checks iff there is a piece in the way of the path it would need to follow.
    int i;
    for (i = 1; i < absolute_value(rank_diff); i++)
    {
        current_rank += rank_step;
        // File does not change for vertical moves
        if (board[current_rank][current_file] != SQUARE_EMPTY)
        {
            return 0;
        }
    }
    return 1;
}

// Forward declaration
static int can_pawn_reach_destination(int from_file, int from_rank,
                                       int to_file, int to_rank,
                                       const struct chess_board *board,
                                       enum chess_player player);

// Helper: Check if a piece movement pattern allows reaching destination
static int can_piece_reach_pattern(enum chess_piece piece,
                                    int from_file, int from_rank,
                                    int to_file, int to_rank,
                                    const struct chess_board *board,
                                    enum chess_player player)
{
    int file_diff = to_file - from_file;
    int rank_diff = to_rank - from_rank;
    
    switch (piece)
    {
    case PIECE_KNIGHT:
        // Knight: L-shape moves (±1, ±2) or (±2, ±1)
        return ((absolute_value(file_diff) == 1 && absolute_value(rank_diff) == 2) ||
                (absolute_value(file_diff) == 2 && absolute_value(rank_diff) == 1));
    
    case PIECE_BISHOP:
        // Bishop: diagonal only (same |file_diff| = |rank_diff|)
        if (absolute_value(file_diff) != absolute_value(rank_diff))
            return 0;
        return is_path_open_diagonally(board->squares, from_file, from_rank, to_file, to_rank);
    
    case PIECE_ROOK:
        // Rook: horizontal or vertical
        if (file_diff != 0 && rank_diff != 0)
            return 0;
        if (file_diff == 0)
            return is_path_open_vertically(board->squares, from_file, from_rank, to_file, to_rank);
        else
            return is_path_open_horizontally(board->squares, from_file, from_rank, to_file, to_rank);
    
    case PIECE_QUEEN:
        // Queen: bishop or rook pattern
        return (can_piece_reach_pattern(PIECE_BISHOP, from_file, from_rank, to_file, to_rank, board, player) ||
                can_piece_reach_pattern(PIECE_ROOK, from_file, from_rank, to_file, to_rank, board, player));
    
    case PIECE_KING:
        // King: adjacent squares only
        return (absolute_value(file_diff) <= 1 && absolute_value(rank_diff) <= 1);
    
    case PIECE_PAWN:
        // Pawn: use full pawn movement logic
        return can_pawn_reach_destination(from_file, from_rank, to_file, to_rank, board, player);
    
    default:
        return 0;
    }
}

// Helper: Check if pawn can reach destination (full implementation)
static int can_pawn_reach_destination(int from_file, int from_rank,
                                       int to_file, int to_rank,
                                       const struct chess_board *board,
                                       enum chess_player player)
{
    int file_diff = to_file - from_file;
    int rank_diff = to_rank - from_rank;
    enum square_content dest_square = board->squares[to_rank][to_file];
    
    if (player == PLAYER_WHITE)
    {
        // White pawns move forward (increasing rank)
        // Normal forward move: 1 square forward, destination empty
        if (file_diff == 0 && rank_diff == 1 && dest_square == SQUARE_EMPTY)
            return 1;
        
        // Double move from starting rank (rank 1): 2 squares forward, both empty
        if (file_diff == 0 && rank_diff == 2 && from_rank == 1 &&
            dest_square == SQUARE_EMPTY &&
            board->squares[from_rank + 1][from_file] == SQUARE_EMPTY)
            return 1;
        
        // Capture: diagonal forward, destination has opponent piece
        if (absolute_value(file_diff) == 1 && rank_diff == 1 &&
            is_square_contains_piece_of_player(dest_square, PLAYER_BLACK))
            return 1;
        
        // En passant: diagonal forward to en passant target square
        if (absolute_value(file_diff) == 1 && rank_diff == 1 &&
            to_file == board->en_passant_file &&
            to_rank == board->en_passant_rank &&
            board->en_passant_file >= 0)
            return 1;
    }
    else
    {
        // Black pawns move forward (decreasing rank)
        // Normal forward move: 1 square forward, destination empty
        if (file_diff == 0 && rank_diff == -1 && dest_square == SQUARE_EMPTY)
            return 1;
        
        // Double move from starting rank (rank 6): 2 squares forward, both empty
        if (file_diff == 0 && rank_diff == -2 && from_rank == 6 &&
            dest_square == SQUARE_EMPTY &&
            board->squares[from_rank - 1][from_file] == SQUARE_EMPTY)
            return 1;
        
        // Capture: diagonal forward, destination has opponent piece
        if (absolute_value(file_diff) == 1 && rank_diff == -1 &&
            is_square_contains_piece_of_player(dest_square, PLAYER_WHITE))
            return 1;
        
        // En passant: diagonal forward to en passant target square
        if (absolute_value(file_diff) == 1 && rank_diff == -1 &&
            to_file == board->en_passant_file &&
            to_rank == board->en_passant_rank &&
            board->en_passant_file >= 0)
            return 1;
    }
    
    return 0;
}

// Helper: Check if a square is attacked by a specific player
static int is_square_attacked(const struct chess_board *board, int file, int rank,
                              enum chess_player attacker)
{
    int f, r;
    enum square_content square;
    enum chess_piece piece_type;
    int rank_diff, file_diff;
    
    // Check all squares for attacking pieces
    for (r = 0; r < 8; r++)
    {
        for (f = 0; f < 8; f++)
        {
            square = board->squares[r][f];
            
            // Skip if empty or not attacker's piece
            if (!is_square_contains_piece_of_player(square, attacker))
                continue;
            
            piece_type = get_piece_type_from_square(square);
            
            // Check if this piece can attack the target square
            if (can_piece_reach_pattern(piece_type, f, r, file, rank, board, attacker))
            {
                // For pawns, need special check (only diagonal attacks count as attacks)
                if (piece_type == PIECE_PAWN)
                {
                    rank_diff = rank - r;
                    file_diff = absolute_value(file - f);
                    if (attacker == PLAYER_WHITE)
                    {
                        // White pawns attack diagonally forward (up)
                        if (rank_diff == 1 && file_diff == 1)
                            return 1;
                    }
                    else
                    {
                        // Black pawns attack diagonally forward (down)
                        if (rank_diff == -1 && file_diff == 1)
                            return 1;
                    }
                }
                else
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Helper: Find king's position for a player
static void find_king(const struct chess_board *board, enum chess_player player,
                      int *king_file, int *king_rank)
{
    int file, rank;
    enum square_content king_square = (player == PLAYER_WHITE) ?
        SQUARE_WHITE_KING : SQUARE_BLACK_KING;
    
    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 8; file++)
        {
            if (board->squares[rank][file] == king_square)
            {
                *king_file = file;
                *king_rank = rank;
                return;
            }
        }
    }
}

// Helper: Check if a player is in check
static int is_in_check(const struct chess_board *board, enum chess_player player)
{
    int king_file, king_rank;
    enum chess_player opponent = get_opponent_player(player);
    
    find_king(board, player, &king_file, &king_rank);
    return is_square_attacked(board, king_file, king_rank, opponent);
}

// Helper: Try to find any legal move for a player
static int find_any_legal_move(const struct chess_board *board, enum chess_player player)
{
    int file, rank, to_file, to_rank;
    enum square_content square;
    enum square_content saved_from, saved_to;
    int found_move = 0;
    enum chess_piece piece_type;
    struct chess_board temp_board;
    enum chess_player opponent = get_opponent_player(player);
    
    // Try all pieces
    for (rank = 0; rank < 8 && !found_move; rank++)
    {
        for (file = 0; file < 8 && !found_move; file++)
        {
            square = board->squares[rank][file];
            if (!is_square_contains_piece_of_player(square, player))
                continue;
            
            piece_type = get_piece_type_from_square(square);
            
            // Try all destination squares
            for (to_rank = 0; to_rank < 8 && !found_move; to_rank++)
            {
                for (to_file = 0; to_file < 8 && !found_move; to_file++)
                {
                    enum square_content dest_square;
                    
                    
                    // Skip if same square
                    if (file == to_file && rank == to_rank)
                        continue;
                    
                    // Check if piece can reach destination (basic pattern check)
                    if (!can_piece_reach_pattern(piece_type, file, rank, to_file, to_rank, board, player))
                        continue;
                    
                    // Additional validation for specific piece types
                    dest_square = board->squares[to_rank][to_file];
                    
                    // For pawns, validate move is legal (forward or capture)
                    if (piece_type == PIECE_PAWN)
                    {
                        if (!can_pawn_reach_destination(file, rank, to_file, to_rank, board, player))
                            continue;
                    }
                    else
                    {
                        // For non-pawn pieces, check if destination has own piece
                        if (is_square_contains_piece_of_player(dest_square, player))
                            continue;  // Can't capture own piece
                    }
                    
                    // Make temporary move to check if it leaves king in check
                    temp_board = *board;
                    saved_from = temp_board.squares[rank][file];
                    saved_to = temp_board.squares[to_rank][to_file];
                    temp_board.squares[to_rank][to_file] = saved_from;
                    temp_board.squares[rank][file] = SQUARE_EMPTY;
                    
                    // Handle en passant capture in temporary board
                    if (piece_type == PIECE_PAWN && 
                        absolute_value(to_file - file) == 1 &&
                        to_file == board->en_passant_file &&
                        to_rank == board->en_passant_rank &&
                        board->en_passant_file >= 0)
                    {
                        int captured_rank = (player == PLAYER_WHITE) ? to_rank - 1 : to_rank + 1;
                        temp_board.squares[captured_rank][to_file] = SQUARE_EMPTY;
                    }
                    
                    // Check if this leaves king in check
                    if (!is_in_check(&temp_board, player))
                        found_move = 1;
                }
            }
        }
    }
    
    // Also check castling moves if no regular move found
    if (!found_move)
    {
        // Try kingside castling
        if ((player == PLAYER_WHITE && board->white_can_castle_kingside) ||
            (player == PLAYER_BLACK && board->black_can_castle_kingside))
        {
            int base_rank = (player == PLAYER_WHITE) ? 0 : 7;
            if (board->squares[base_rank][5] == SQUARE_EMPTY &&
                board->squares[base_rank][6] == SQUARE_EMPTY &&
                !is_square_attacked(board, 4, base_rank, opponent) &&
                !is_square_attacked(board, 5, base_rank, opponent) &&
                !is_square_attacked(board, 6, base_rank, opponent))
            {
                found_move = 1;
            }
        }
        
        // Try queenside castling
        if (!found_move)
        {
            if ((player == PLAYER_WHITE && board->white_can_castle_queenside) ||
                (player == PLAYER_BLACK && board->black_can_castle_queenside))
            {
                int base_rank = (player == PLAYER_WHITE) ? 0 : 7;
                if (board->squares[base_rank][1] == SQUARE_EMPTY &&
                    board->squares[base_rank][2] == SQUARE_EMPTY &&
                    board->squares[base_rank][3] == SQUARE_EMPTY &&
                    !is_square_attacked(board, 4, base_rank, opponent) &&
                    !is_square_attacked(board, 3, base_rank, opponent) &&
                    !is_square_attacked(board, 2, base_rank, opponent))
                {
                    found_move = 1;
                }
            }
        }
    }
    
    return found_move;
}

void board_initialize(struct chess_board *board)
{
    board->next_move_player = PLAYER_WHITE;
    
    // Initialize all squares to empty
    int rank, file;
    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 8; file++)
        {
            board->squares[rank][file] = SQUARE_EMPTY;
        }
    }
    
    // White back rank (rank 0): Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook
    board->squares[0][0] = SQUARE_WHITE_ROOK;
    board->squares[0][1] = SQUARE_WHITE_KNIGHT;
    board->squares[0][2] = SQUARE_WHITE_BISHOP;
    board->squares[0][3] = SQUARE_WHITE_QUEEN;
    board->squares[0][4] = SQUARE_WHITE_KING;
    board->squares[0][5] = SQUARE_WHITE_BISHOP;
    board->squares[0][6] = SQUARE_WHITE_KNIGHT;
    board->squares[0][7] = SQUARE_WHITE_ROOK;
    
    // White pawns (rank 1)
    for (file = 0; file < 8; file++)
    {
        board->squares[1][file] = SQUARE_WHITE_PAWN;
    }
    
    // Black back rank (rank 7): Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook
    board->squares[7][0] = SQUARE_BLACK_ROOK;
    board->squares[7][1] = SQUARE_BLACK_KNIGHT;
    board->squares[7][2] = SQUARE_BLACK_BISHOP;
    board->squares[7][3] = SQUARE_BLACK_QUEEN;
    board->squares[7][4] = SQUARE_BLACK_KING;
    board->squares[7][5] = SQUARE_BLACK_BISHOP;
    board->squares[7][6] = SQUARE_BLACK_KNIGHT;
    board->squares[7][7] = SQUARE_BLACK_ROOK;
    
    // Black pawns (rank 6)
    for (file = 0; file < 8; file++)
    {
        board->squares[6][file] = SQUARE_BLACK_PAWN;
    }
    
    // Initialize castling rights - both players can castle both ways at start
    board->white_can_castle_kingside = 1;
    board->white_can_castle_queenside = 1;
    board->black_can_castle_kingside = 1;
    board->black_can_castle_queenside = 1;
    
    // Initialize en passant target - no target at start
    board->en_passant_file = -1;
    board->en_passant_rank = -1;
}

void board_complete_move(const struct chess_board *board, struct chess_move *move)
{
    int file, rank;
    int candidate_file = -1, candidate_rank = -1;
    int candidate_count = 0;
    enum square_content square;
    enum square_content dest_square;
    enum chess_player opponent;
    
    // Set which player is making this move
    move->player = board->next_move_player;
    
    // Handle castling moves - starting position is always known
    if (move->is_castle_kingside || move->is_castle_queenside)
    {
        // King's starting position is always e1 (white) or e8 (black)
        if (move->player == PLAYER_WHITE)
        {
            move->from_file = 4;  // e file
            move->from_rank = 0;   // rank 1
        }
        else
        {
            move->from_file = 4;  // e file
            move->from_rank = 7;   // rank 8
        }
        // Set destination for castling
        if (move->is_castle_kingside)
        {
            move->to_file = 6;  // g file
            move->to_rank = move->from_rank;
        }
        else
        {
            move->to_file = 2;  // c file
            move->to_rank = move->from_rank;
        }
        return;  // Castling moves are now complete
    }
    
    // If both from_file and from_rank are already known, move is complete
    if (move->from_file >= 0 && move->from_rank >= 0)
        return;
    
    // Search the board for pieces that could make this move
    opponent = get_opponent_player(move->player);
    
    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 8; file++)
        {
            square = board->squares[rank][file];
            
            // Skip if square is empty or doesn't have a piece of the correct color
            if (!is_square_contains_piece_of_player(square, move->player))
                continue;
            
            // Check if piece type matches
            if (get_piece_type_from_square(square) != move->piece_type)
                continue;
            
            // Apply disambiguation constraints
            // If from_file was specified, this piece must be on that file
            if (move->from_file >= 0 && file != move->from_file)
                continue;
            
            // If from_rank was specified, this piece must be on that rank
            if (move->from_rank >= 0 && rank != move->from_rank)
                continue;
            
            // Check if this piece can reach the destination (movement pattern only)
            if (!can_piece_reach_pattern(move->piece_type, file, rank,
                                         move->to_file, move->to_rank, board, move->player))
                continue;
            
            // Check capture/non-capture constraint
            dest_square = board->squares[move->to_rank][move->to_file];
            
            // Special handling for pawns and en passant
            if (move->piece_type == PIECE_PAWN)
            {
                // For pawns, check if it's a valid pawn move (which handles captures too)
                if (!can_pawn_reach_destination(file, rank, move->to_file, move->to_rank, board, move->player))
                    continue;
                
                // Check if it's an en passant capture
                if (absolute_value(move->to_file - file) == 1 &&
                    move->to_file == board->en_passant_file &&
                    move->to_rank == board->en_passant_rank &&
                    board->en_passant_file >= 0)
                {
                    // This is an en passant capture
                    if (move->is_capture)
                    {
                        move->is_en_passant = 1;
                    }
                    else
                    {
                        continue;  // En passant must be a capture
                    }
                }
                else if (move->is_capture)
                {
                    // Regular pawn capture - destination must have opponent piece
                    if (!is_square_contains_piece_of_player(dest_square, opponent))
                        continue;
                }
                else
                {
                    // Normal pawn move - destination must be empty
                    if (dest_square != SQUARE_EMPTY)
                        continue;
                }
            }
            else
            {
                // For non-pawn pieces
                if (move->is_capture)
                {
                    // Destination must have opponent piece
                    if (!is_square_contains_piece_of_player(dest_square, opponent))
                        continue;
                }
                else
                {
                    // Destination must be empty
                    if (dest_square != SQUARE_EMPTY)
                        continue;
                }
            }
            
            // This is a candidate piece that could make the move!
            candidate_count++;
            candidate_file = file;
            candidate_rank = rank;
        }
    }
    
    // Handle the results
    if (candidate_count == 0)
    {
        // No piece can make this move - error!
        panicf("move completion error: %s %s to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->to_file,
               move->to_rank + 1);
    }
    else if (candidate_count > 1)
    {
        // Multiple pieces could make this move - ambiguous, error!
        panicf("move completion error: %s %s to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->to_file,
               move->to_rank + 1);
    }
    else
    {
        // Exactly one candidate - complete the move!
        move->from_file = candidate_file;
        move->from_rank = candidate_rank;
    }
}

void board_apply_move(struct chess_board *board, const struct chess_move *move)
{
    enum square_content piece;
    enum square_content *from_square, *to_square;
    enum square_content saved_from, saved_to;
    enum chess_player opponent;
    
    // Verify move is complete (from_file and from_rank must be known)
    if (move->from_file < 0 || move->from_rank < 0)
    {
        panicf("illegal move: %s %s from unknown to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->to_file,
               move->to_rank + 1);
    }
    
    // Get pointers to the squares involved
    from_square = &board->squares[move->from_rank][move->from_file];
    to_square = &board->squares[move->to_rank][move->to_file];
    piece = *from_square;
    opponent = get_opponent_player(move->player);
    
    // Verify piece exists and is correct color
    if (!is_square_contains_piece_of_player(piece, move->player))
    {
        panicf("illegal move: %s %s from %c%d to %c%d\n",
               player_string(move->player),
               piece_string(move->piece_type),
               'a' + move->from_file, move->from_rank + 1,
               'a' + move->to_file, move->to_rank + 1);
    }
    
    // Handle castling separately
    if (move->is_castle_kingside || move->is_castle_queenside)
    {
        // Validate castling conditions
        if (move->player == PLAYER_WHITE)
        {
            if ((move->is_castle_kingside && !board->white_can_castle_kingside) ||
                (move->is_castle_queenside && !board->white_can_castle_queenside))
            {
                panicf("illegal move: %s king from %c%d to %c%d",
                       player_string(move->player),
                       'a' + move->from_file, move->from_rank + 1,
                       'a' + move->to_file, move->to_rank + 1);
            }
            
            // Check squares are empty and not attacked
            if (move->is_castle_kingside)
            {
                // Check f1 and g1 are empty, e1, f1 and g1 not attacked
                if (board->squares[0][5] != SQUARE_EMPTY ||
                    board->squares[0][6] != SQUARE_EMPTY ||
                    is_square_attacked(board, 4, 0, opponent) ||
                    is_square_attacked(board, 5, 0, opponent) ||
                    is_square_attacked(board, 6, 0, opponent))
                {
                    panicf("illegal move: %s king from %c%d to %c%d\n",
                           player_string(move->player),
                           'a' + move->from_file, move->from_rank + 1,
                           'a' + move->to_file, move->to_rank + 1);
                }
            }
            else
            {
                // Check b1, c1, d1 are empty, c1, d1 not attacked
                if (board->squares[0][1] != SQUARE_EMPTY ||
                    board->squares[0][2] != SQUARE_EMPTY ||
                    board->squares[0][3] != SQUARE_EMPTY ||
                    is_square_attacked(board, 4, 0, opponent) ||
                    is_square_attacked(board, 3, 0, opponent) ||
                    is_square_attacked(board, 2, 0, opponent))
                {
                    panicf("illegal move: %s king from %c%d to %c%d\n",
                           player_string(move->player),
                           'a' + move->from_file, move->from_rank + 1,
                           'a' + move->to_file, move->to_rank + 1);
                }
            }
        }
        else  // black
        {
            if ((move->is_castle_kingside && !board->black_can_castle_kingside) ||
                (move->is_castle_queenside && !board->black_can_castle_queenside))
            {
                panicf("illegal move: %s king from %c%d to %c%d\n",
                       player_string(move->player),
                       'a' + move->from_file, move->from_rank + 1,
                       'a' + move->to_file, move->to_rank + 1);
            }
            
            // Check squares are empty and not attacked
            if (move->is_castle_kingside)
            {
                // Check f8 and g8 are empty, e8, f8 and g8 not attacked
                if (board->squares[7][5] != SQUARE_EMPTY ||
                    board->squares[7][6] != SQUARE_EMPTY ||
                    is_square_attacked(board, 4, 7, opponent) ||
                    is_square_attacked(board, 5, 7, opponent) ||
                    is_square_attacked(board, 6, 7, opponent))
                {
                    panicf("illegal move: %s king from %c%d to %c%d\n",
                           player_string(move->player),
                           'a' + move->from_file, move->from_rank + 1,
                           'a' + move->to_file, move->to_rank + 1);
                }
            }
            else
            {
                // Check b8, c8, d8 are empty, c8, d8 not attacked
                if (board->squares[7][1] != SQUARE_EMPTY ||
                    board->squares[7][2] != SQUARE_EMPTY ||
                    board->squares[7][3] != SQUARE_EMPTY ||
                    is_square_attacked(board, 4, 7, opponent) ||
                    is_square_attacked(board, 3, 7, opponent) ||
                    is_square_attacked(board, 2, 7, opponent))
                {
                    panicf("illegal move: %s king from %c%d to %c%d\n",
                           player_string(move->player),
                           'a' + move->from_file, move->from_rank + 1,
                           'a' + move->to_file, move->to_rank + 1);
                }
            }
        }
        
        // Apply castling move
        if (move->is_castle_kingside)
        {
            // Move king and rook
            if (move->player == PLAYER_WHITE)
            {
                board->squares[0][6] = SQUARE_WHITE_KING;  // g1
                board->squares[0][5] = SQUARE_WHITE_ROOK;  // f1
                board->squares[0][4] = SQUARE_EMPTY;       // e1
                board->squares[0][7] = SQUARE_EMPTY;       // h1
            }
            else
            {
                board->squares[7][6] = SQUARE_BLACK_KING;  // g8
                board->squares[7][5] = SQUARE_BLACK_ROOK;  // f8
                board->squares[7][4] = SQUARE_EMPTY;       // e8
                board->squares[7][7] = SQUARE_EMPTY;       // h8
            }
        }
        else  // queenside
        {
            // Move king and rook
            if (move->player == PLAYER_WHITE)
            {
                board->squares[0][2] = SQUARE_WHITE_KING;  // c1
                board->squares[0][3] = SQUARE_WHITE_ROOK;  // d1
                board->squares[0][4] = SQUARE_EMPTY;       // e1
                board->squares[0][0] = SQUARE_EMPTY;       // a1
            }
            else
            {
                board->squares[7][2] = SQUARE_BLACK_KING;  // c8
                board->squares[7][3] = SQUARE_BLACK_ROOK;  // d8
                board->squares[7][4] = SQUARE_EMPTY;       // e8
                board->squares[7][0] = SQUARE_EMPTY;       // a8
            }
        }
        
        // Update castling flags
        if (move->player == PLAYER_WHITE)
        {
            board->white_can_castle_kingside = 0;
            board->white_can_castle_queenside = 0;
        }
        else
        {
            board->black_can_castle_kingside = 0;
            board->black_can_castle_queenside = 0;
        }
        
        // Clear en passant (castling doesn't create en passant)
        board->en_passant_file = -1;
        board->en_passant_rank = -1;
    }
    else
    {
        // Regular move - check if it would leave king in check
        // Make temporary move
        saved_from = *from_square;
        saved_to = *to_square;
        *to_square = piece;
        *from_square = SQUARE_EMPTY;
        
        // Check if this leaves our own king in check
        if (is_in_check(board, move->player))
        {
            // Undo temporary move
            *from_square = saved_from;
            *to_square = saved_to;
            panicf("illegal move: %s %s from %c%d to %c%d\n",
                   player_string(move->player),
                   piece_string(move->piece_type),
                   'a' + move->from_file, move->from_rank + 1,
                   'a' + move->to_file, move->to_rank + 1);
        }
        
        // Handle en passant capture
        if (move->is_en_passant)
        {
            // Remove captured pawn (behind destination square)
            int captured_rank = (move->player == PLAYER_WHITE) ?
                move->to_rank - 1 : move->to_rank + 1;
            board->squares[captured_rank][move->to_file] = SQUARE_EMPTY;
        }
        
        // Handle promotion
        if (move->promotion_piece != PIECE_PAWN)
        {
            // Replace pawn with promoted piece
            if (move->player == PLAYER_WHITE)
            {
                switch (move->promotion_piece)
                {
                case PIECE_QUEEN: piece = SQUARE_WHITE_QUEEN; break;
                case PIECE_ROOK: piece = SQUARE_WHITE_ROOK; break;
                case PIECE_BISHOP: piece = SQUARE_WHITE_BISHOP; break;
                case PIECE_KNIGHT: piece = SQUARE_WHITE_KNIGHT; break;
                default: break;
                }
            }
            else
            {
                switch (move->promotion_piece)
                {
                case PIECE_QUEEN: piece = SQUARE_BLACK_QUEEN; break;
                case PIECE_ROOK: piece = SQUARE_BLACK_ROOK; break;
                case PIECE_BISHOP: piece = SQUARE_BLACK_BISHOP; break;
                case PIECE_KNIGHT: piece = SQUARE_BLACK_KNIGHT; break;
                default: break;
                }
            }
        }
        
        // Apply the move permanently (already done temporarily above)
        *to_square = piece;
        *from_square = SQUARE_EMPTY;
        
        // Update castling flags if king or rook moved
        if (move->piece_type == PIECE_KING)
        {
            if (move->player == PLAYER_WHITE)
            {
                board->white_can_castle_kingside = 0;
                board->white_can_castle_queenside = 0;
            }
            else
            {
                board->black_can_castle_kingside = 0;
                board->black_can_castle_queenside = 0;
            }
        }
        else if (move->piece_type == PIECE_ROOK)
        {
            // Check if it's a rook that moved from starting position
            if (move->player == PLAYER_WHITE && move->from_rank == 0)
            {
                if (move->from_file == 0)
                    board->white_can_castle_queenside = 0;
                else if (move->from_file == 7)
                    board->white_can_castle_kingside = 0;
            }
            else if (move->player == PLAYER_BLACK && move->from_rank == 7)
        
                    // Also disable castling if a rook is captured on its starting square
                    if (move->is_capture)
                    {
                        // Check if captured piece was on a rook starting square
                        if (move->to_rank == 0)  // White's back rank
                        {
                            if (move->to_file == 0)  // a1 - white queenside rook
                                board->white_can_castle_queenside = 0;
                            else if (move->to_file == 7)  // h1 - white kingside rook
                                board->white_can_castle_kingside = 0;
                        }
                        else if (move->to_rank == 7)  // Black's back rank
                        {
                            if (move->to_file == 0)  // a8 - black queenside rook
                                board->black_can_castle_queenside = 0;
                            else if (move->to_file == 7)  // h8 - black kingside rook
                                board->black_can_castle_kingside = 0;
                        }
                    }
            {
                if (move->from_file == 0)
                    board->black_can_castle_queenside = 0;
                else if (move->from_file == 7)
                    board->black_can_castle_kingside = 0;
            }
        }
        
        // Update en passant target (if pawn moved two squares)
        if (move->piece_type == PIECE_PAWN &&
            absolute_value(move->to_rank - move->from_rank) == 2)
        {
            // Set en passant target to square behind destination
            board->en_passant_file = move->to_file;
            board->en_passant_rank = (move->from_rank + move->to_rank) / 2;
        }
        else
        {
            // Clear en passant target
            board->en_passant_file = -1;
            board->en_passant_rank = -1;
        }
    }
    
    // The final step is to update the the turn of players in the board state.
    switch (board->next_move_player)
    {
    case PLAYER_WHITE:
        board->next_move_player = PLAYER_BLACK;
        break;
    case PLAYER_BLACK:
        board->next_move_player = PLAYER_WHITE;
        break;
    }
}

void board_summarize(const struct chess_board *board)
{
    enum chess_player current_player = board->next_move_player;
    int in_check = is_in_check(board, current_player);
    int has_legal_moves = find_any_legal_move(board, current_player);
    
    if (in_check && !has_legal_moves)
    {
        // Checkmate - opponent wins
        enum chess_player winner = get_opponent_player(current_player);
        if (winner == PLAYER_WHITE)
        {
            printf("white wins by checkmate\n");
        }
        else
        {
            printf("black wins by checkmate\n");
        }
    }
    else if (!in_check && !has_legal_moves)
    {
        // Stalemate - draw
        printf("draw by stalemate\n");
    }
    else
    {
        // Game is still in progress
        printf("game incomplete\n");
    }
}
