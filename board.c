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
    return square == SQUARE_WHITE_PAWN || square == SQUARE_WHITE_KNIGHT || square == SQUARE_WHITE_BISHOP || square == SQUARE_WHITE_ROOK || square == SQUARE_WHITE_QUEEN || square == SQUARE_WHITE_KING;
    if (square == SQUARE_BLACK_PAWN || square == SQUARE_BLACK_KNIGHT || square == SQUARE_BLACK_BISHOP || square == SQUARE_BLACK_ROOK || square == SQUARE_BLACK_QUEEN || square == SQUARE_BLACK_KING)
    {
        return 1;
    }
    return 0;
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
        panic("Invalid square content for getting piece type.");
    }
}
void board_initialize(struct chess_board *board)
{
    board->next_move_player = PLAYER_WHITE;
    // TODO: initialize the board state for a new chess game.

}

void board_complete_move(const struct chess_board *board, struct chess_move *move)
{
    // TODO: complete the move.
}

void board_apply_move(struct chess_board *board, const struct chess_move *move)
{
    // TODO: apply a completed move to the board.

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
    // TODO: print the state of the game.
}
