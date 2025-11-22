#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

/* Chess piece definitions */
#define EMPTY   ' '
#define PAWN_W  'P'
#define PAWN_B  'p'
#define ROOK_W  'R'
#define ROOK_B  'r'
#define KNIGHT_W 'N'
#define KNIGHT_B 'n'
#define BISHOP_W 'B'
#define BISHOP_B 'b'
#define QUEEN_W  'Q'
#define QUEEN_B  'q'
#define KING_W   'K'
#define KING_B   'k'

/* Board dimensions */
#define BOARD_SIZE 8

/* Color definitions */
typedef enum {
    WHITE,
    BLACK
} Color;

/* Move structure */
typedef struct {
    int from_row;
    int from_col;
    int to_row;
    int to_col;
    char piece;
    char captured;
} Move;

/* Board structure */
typedef struct {
    char squares[BOARD_SIZE][BOARD_SIZE];
    Color turn;
    bool white_king_moved;
    bool black_king_moved;
    bool white_rook_a_moved;
    bool white_rook_h_moved;
    bool black_rook_a_moved;
    bool black_rook_h_moved;
} Board;

/* Function prototypes */
void init_board(Board *board);
void print_board(const Board *board);
bool is_valid_position(int row, int col);
bool is_white_piece(char piece);
bool is_black_piece(char piece);
Color get_piece_color(char piece);
bool is_valid_move(const Board *board, const Move *move);
bool make_move(Board *board, const Move *move);
bool is_in_check(const Board *board, Color color);
bool is_checkmate(const Board *board, Color color);
bool is_stalemate(const Board *board, Color color);

#endif /* BOARD_H */
