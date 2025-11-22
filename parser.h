#ifndef PARSER_H
#define PARSER_H

#include "board.h"

/* Parse a move in algebraic notation (e.g., "e2e4") */
bool parse_move(const char *move_str, Move *move, const Board *board);

/* Convert column letter to index (a-h -> 0-7) */
int col_to_index(char col);

/* Convert row number to index (1-8 -> 7-0) */
int row_to_index(char row);

/* Convert index to column letter (0-7 -> a-h) */
char index_to_col(int col);

/* Convert index to row number (7-0 -> 1-8) */
char index_to_row(int row);

#endif /* PARSER_H */
