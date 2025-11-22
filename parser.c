#include "parser.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Convert column letter to index */
int col_to_index(char col) {
    if (col >= 'a' && col <= 'h') {
        return col - 'a';
    }
    if (col >= 'A' && col <= 'H') {
        return col - 'A';
    }
    return -1;
}

/* Convert row number to index */
int row_to_index(char row) {
    if (row >= '1' && row <= '8') {
        return BOARD_SIZE - (row - '0');
    }
    return -1;
}

/* Convert index to column letter */
char index_to_col(int col) {
    if (col >= 0 && col < BOARD_SIZE) {
        return 'a' + col;
    }
    return '?';
}

/* Convert index to row number */
char index_to_row(int row) {
    if (row >= 0 && row < BOARD_SIZE) {
        return '0' + (BOARD_SIZE - row);
    }
    return '?';
}

/* Parse a move in algebraic notation */
bool parse_move(const char *move_str, Move *move, const Board *board) {
    if (move_str == NULL || move == NULL || board == NULL) {
        return false;
    }
    
    size_t len = strlen(move_str);
    
    /* Basic format: e2e4 (4 characters) */
    if (len < 4) {
        return false;
    }
    
    /* Parse source position */
    int from_col = col_to_index(move_str[0]);
    int from_row = row_to_index(move_str[1]);
    
    /* Parse destination position */
    int to_col = col_to_index(move_str[2]);
    int to_row = row_to_index(move_str[3]);
    
    /* Validate positions */
    if (from_col == -1 || from_row == -1 || to_col == -1 || to_row == -1) {
        return false;
    }
    
    /* Get the piece at source position */
    char piece = board->squares[from_row][from_col];
    
    if (piece == EMPTY) {
        return false;
    }
    
    /* Get captured piece if any */
    char captured = board->squares[to_row][to_col];
    
    /* Fill in move structure */
    move->from_row = from_row;
    move->from_col = from_col;
    move->to_row = to_row;
    move->to_col = to_col;
    move->piece = piece;
    move->captured = captured;
    
    return true;
}
