#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

/* Initialize the board with starting position */
void init_board(Board *board) {
    /* Set up pawns */
    for (int col = 0; col < BOARD_SIZE; col++) {
        board->squares[1][col] = PAWN_B;
        board->squares[6][col] = PAWN_W;
    }
    
    /* Set up back ranks */
    board->squares[0][0] = ROOK_B;
    board->squares[0][1] = KNIGHT_B;
    board->squares[0][2] = BISHOP_B;
    board->squares[0][3] = QUEEN_B;
    board->squares[0][4] = KING_B;
    board->squares[0][5] = BISHOP_B;
    board->squares[0][6] = KNIGHT_B;
    board->squares[0][7] = ROOK_B;
    
    board->squares[7][0] = ROOK_W;
    board->squares[7][1] = KNIGHT_W;
    board->squares[7][2] = BISHOP_W;
    board->squares[7][3] = QUEEN_W;
    board->squares[7][4] = KING_W;
    board->squares[7][5] = BISHOP_W;
    board->squares[7][6] = KNIGHT_W;
    board->squares[7][7] = ROOK_W;
    
    /* Set up empty squares */
    for (int row = 2; row < 6; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            board->squares[row][col] = EMPTY;
        }
    }
    
    /* Initialize game state */
    board->turn = WHITE;
    board->white_king_moved = false;
    board->black_king_moved = false;
    board->white_rook_a_moved = false;
    board->white_rook_h_moved = false;
    board->black_rook_a_moved = false;
    board->black_rook_h_moved = false;
}

/* Print the board to stdout */
void print_board(const Board *board) {
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    for (int row = 0; row < BOARD_SIZE; row++) {
        printf("%d |", 8 - row);
        for (int col = 0; col < BOARD_SIZE; col++) {
            printf(" %c |", board->squares[row][col]);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n\n");
}

/* Check if position is valid */
bool is_valid_position(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

/* Check if piece is white */
bool is_white_piece(char piece) {
    return piece >= 'A' && piece <= 'Z';
}

/* Check if piece is black */
bool is_black_piece(char piece) {
    return piece >= 'a' && piece <= 'z';
}

/* Get color of a piece */
Color get_piece_color(char piece) {
    if (is_white_piece(piece)) {
        return WHITE;
    }
    return BLACK;
}

/* Check if a pawn move is valid */
static bool is_valid_pawn_move(const Board *board, const Move *move) {
    int direction = (move->piece == PAWN_W) ? -1 : 1;
    int start_row = (move->piece == PAWN_W) ? 6 : 1;
    
    /* Forward move */
    if (move->from_col == move->to_col) {
        /* One square forward */
        if (move->to_row == move->from_row + direction && 
            board->squares[move->to_row][move->to_col] == EMPTY) {
            return true;
        }
        /* Two squares forward from starting position */
        if (move->from_row == start_row && 
            move->to_row == move->from_row + 2 * direction &&
            board->squares[move->from_row + direction][move->from_col] == EMPTY &&
            board->squares[move->to_row][move->to_col] == EMPTY) {
            return true;
        }
    }
    
    /* Diagonal capture */
    if (abs(move->from_col - move->to_col) == 1 && 
        move->to_row == move->from_row + direction) {
        char target = board->squares[move->to_row][move->to_col];
        if (target != EMPTY) {
            Color piece_color = get_piece_color(move->piece);
            Color target_color = get_piece_color(target);
            return piece_color != target_color;
        }
    }
    
    return false;
}

/* Check if a knight move is valid */
static bool is_valid_knight_move(const Board *board, const Move *move) {
    (void)board;  /* Unused parameter */
    int row_diff = abs(move->to_row - move->from_row);
    int col_diff = abs(move->to_col - move->from_col);
    
    return (row_diff == 2 && col_diff == 1) || (row_diff == 1 && col_diff == 2);
}

/* Check if a bishop move is valid */
static bool is_valid_bishop_move(const Board *board, const Move *move) {
    int row_diff = abs(move->to_row - move->from_row);
    int col_diff = abs(move->to_col - move->from_col);
    
    if (row_diff != col_diff) {
        return false;
    }
    
    /* Check path is clear */
    int row_dir = (move->to_row > move->from_row) ? 1 : -1;
    int col_dir = (move->to_col > move->from_col) ? 1 : -1;
    
    int row = move->from_row + row_dir;
    int col = move->from_col + col_dir;
    
    while (row != move->to_row) {
        if (board->squares[row][col] != EMPTY) {
            return false;
        }
        row += row_dir;
        col += col_dir;
    }
    
    return true;
}

/* Check if a rook move is valid */
static bool is_valid_rook_move(const Board *board, const Move *move) {
    if (move->from_row != move->to_row && move->from_col != move->to_col) {
        return false;
    }
    
    /* Check path is clear */
    if (move->from_row == move->to_row) {
        int start = (move->from_col < move->to_col) ? move->from_col : move->to_col;
        int end = (move->from_col < move->to_col) ? move->to_col : move->from_col;
        
        for (int col = start + 1; col < end; col++) {
            if (board->squares[move->from_row][col] != EMPTY) {
                return false;
            }
        }
    } else {
        int start = (move->from_row < move->to_row) ? move->from_row : move->to_row;
        int end = (move->from_row < move->to_row) ? move->to_row : move->from_row;
        
        for (int row = start + 1; row < end; row++) {
            if (board->squares[row][move->from_col] != EMPTY) {
                return false;
            }
        }
    }
    
    return true;
}

/* Check if a queen move is valid */
static bool is_valid_queen_move(const Board *board, const Move *move) {
    return is_valid_rook_move(board, move) || is_valid_bishop_move(board, move);
}

/* Check if a king move is valid */
static bool is_valid_king_move(const Board *board, const Move *move) {
    (void)board;  /* Unused parameter */
    int row_diff = abs(move->to_row - move->from_row);
    int col_diff = abs(move->to_col - move->from_col);
    
    return row_diff <= 1 && col_diff <= 1;
}

/* Helper function to check if a move is valid according to piece rules, ignoring turn */
static bool is_move_legal_for_piece(const Board *board, const Move *move) {
    /* Check positions are valid */
    if (!is_valid_position(move->from_row, move->from_col) ||
        !is_valid_position(move->to_row, move->to_col)) {
        return false;
    }
    
    /* Check there's a piece at from position */
    if (board->squares[move->from_row][move->from_col] == EMPTY) {
        return false;
    }
    
    /* Check piece matches */
    if (board->squares[move->from_row][move->from_col] != move->piece) {
        return false;
    }
    
    /* Check destination */
    Color piece_color = get_piece_color(move->piece);
    char target = board->squares[move->to_row][move->to_col];
    if (target != EMPTY) {
        Color target_color = get_piece_color(target);
        if (target_color == piece_color) {
            return false;  /* Can't capture own piece */
        }
    }
    
    /* Check piece-specific move rules */
    char piece_type = toupper(move->piece);
    
    switch (piece_type) {
        case 'P':
            return is_valid_pawn_move(board, move);
        case 'N':
            return is_valid_knight_move(board, move);
        case 'B':
            return is_valid_bishop_move(board, move);
        case 'R':
            return is_valid_rook_move(board, move);
        case 'Q':
            return is_valid_queen_move(board, move);
        case 'K':
            return is_valid_king_move(board, move);
        default:
            return false;
    }
}

/* Check if a move is valid according to piece rules */
bool is_valid_move(const Board *board, const Move *move) {
    /* Check it's the right player's turn */
    Color piece_color = get_piece_color(move->piece);
    if (piece_color != board->turn) {
        return false;
    }
    
    return is_move_legal_for_piece(board, move);
}

/* Make a move on the board */
bool make_move(Board *board, const Move *move) {
    if (!is_valid_move(board, move)) {
        return false;
    }
    
    /* Update castling rights */
    if (move->piece == KING_W) {
        board->white_king_moved = true;
    } else if (move->piece == KING_B) {
        board->black_king_moved = true;
    } else if (move->piece == ROOK_W) {
        if (move->from_col == 0) board->white_rook_a_moved = true;
        if (move->from_col == 7) board->white_rook_h_moved = true;
    } else if (move->piece == ROOK_B) {
        if (move->from_col == 0) board->black_rook_a_moved = true;
        if (move->from_col == 7) board->black_rook_h_moved = true;
    }
    
    /* Make the move */
    board->squares[move->to_row][move->to_col] = move->piece;
    board->squares[move->from_row][move->from_col] = EMPTY;
    
    /* Switch turn */
    board->turn = (board->turn == WHITE) ? BLACK : WHITE;
    
    return true;
}

/* Check if a king is in check */
bool is_in_check(const Board *board, Color color) {
    /* Find king position */
    char king = (color == WHITE) ? KING_W : KING_B;
    int king_row = -1, king_col = -1;
    
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board->squares[row][col] == king) {
                king_row = row;
                king_col = col;
                break;
            }
        }
        if (king_row != -1) break;
    }
    
    if (king_row == -1) {
        return false;  /* King not found */
    }
    
    /* Check if any opponent piece can capture the king */
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            char piece = board->squares[row][col];
            if (piece != EMPTY && get_piece_color(piece) != color) {
                Move test_move = {
                    .from_row = row,
                    .from_col = col,
                    .to_row = king_row,
                    .to_col = king_col,
                    .piece = piece,
                    .captured = king
                };
                
                /* Check if this piece can attack the king (ignoring turn) */
                if (is_move_legal_for_piece(board, &test_move)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

/* Check if a position is checkmate */
bool is_checkmate(const Board *board, Color color) {
    if (!is_in_check(board, color)) {
        return false;
    }
    
    /* Try all possible moves to see if any gets out of check */
    /* This is a simplified implementation */
    return false;  /* Placeholder */
}

/* Check if a position is stalemate */
bool is_stalemate(const Board *board, Color color) {
    if (is_in_check(board, color)) {
        return false;
    }
    
    /* Check if there are any legal moves */
    /* This is a simplified implementation */
    return false;  /* Placeholder */
}
