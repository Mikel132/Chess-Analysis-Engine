#include "parser.h"
#include <stdio.h>
#include "panic.h"

bool parse_move(struct chess_move *move)
{
    char c;
    int file, rank;


    // Get the first character of the move, ignoring any initial spaces.
    do
    {
        c = getc(stdin);
    } while (c == ' ');

    // Check if we are at the end of input.
    if (c == '\n' || c == '\r')
        return false;

        // This initializes all the fields of the move structure to their default values.
        move->from_file = -1;
        move->from_rank = -1;
        move->to_file = -1;
        move->to_rank = -1;
        move->is_capture = 0;
        move->is_castle_kingside = 0;
        move->is_castle_queenside = 0;
        move->is_en_passant = 0;

    switch (c)
    {
    // TODO: parse the move, starting from the first character. You are free to
    // start from this switch/case as a template or use a different approach.
    default:
        panicf("parse error at character '%c'\n", c);
    }
}
