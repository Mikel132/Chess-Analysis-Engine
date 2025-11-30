#include "parser.h"
#include <stdio.h>
#include "panic.h"

// Helper function to parse a file character (a-h) and return 0-7
static int parse_file(char c)
{
    if (c >= 'a' && c <= 'h')
        return c - 'a';
    return -1;
}

// Helper function to parse a rank character (1-8) and return 0-7
static int parse_rank(char c)
{
    if (c >= '1' && c <= '8')
        return c - '1';
    return -1;
}

// Helper function to parse a piece letter and return the piece type
static enum chess_piece parse_piece_letter(char c)
{
    switch (c)
    {
    case 'K':
        return PIECE_KING;
    case 'Q':
        return PIECE_QUEEN;
    case 'R':
        return PIECE_ROOK;
    case 'B':
        return PIECE_BISHOP;
    case 'N':
        return PIECE_KNIGHT;
    default:
        return PIECE_PAWN; // Not a valid piece letter
    }
}

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
    move->promotion_piece = PIECE_PAWN;

    switch (c)
    {
    case 'O':
        // Castling: O-O (kingside) or O-O-O (queenside)
        move->piece_type = PIECE_KING;
        c = getc(stdin);
        if (c != '-')
            panicf("parse error at character '%c'\n", c);
        c = getc(stdin);
        if (c != 'O')
            panicf("parse error at character '%c'\n", c);
        // Now check for O-O-O (queenside) or just O-O (kingside)
        c = getc(stdin);
        if (c == '-')
        {
            // O-O-O (queenside)
            c = getc(stdin);
            if (c != 'O')
                panicf("parse error at character '%c'\n", c);
            move->is_castle_queenside = 1;
        }
        else
        {
            // O-O (kingside) - put back the extra character
            ungetc(c, stdin);
            move->is_castle_kingside = 1;
        }
        return true;

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
        // Pawn move: starts with file letter
        // Patterns: e4, exd5, e8=Q, exd8=Q
        move->piece_type = PIECE_PAWN;
        file = parse_file(c);

        c = getc(stdin);
        if (c == 'x')
        {
            // Capture: the file we read is the from_file
            move->is_capture = 1;
            move->from_file = file;
            c = getc(stdin);
            file = parse_file(c);
            if (file < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_file = file;
            c = getc(stdin);
            rank = parse_rank(c);
            if (rank < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_rank = rank;
        }
        else
        {
            // Non-capture: file is destination file
            rank = parse_rank(c);
            if (rank < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_file = file;
            move->to_rank = rank;
        }

        // Check for promotion
        c = getc(stdin);
        if (c == '=')
        {
            c = getc(stdin);
            switch (c)
            {
            case 'Q':
                move->promotion_piece = PIECE_QUEEN;
                break;
            case 'R':
                move->promotion_piece = PIECE_ROOK;
                break;
            case 'B':
                move->promotion_piece = PIECE_BISHOP;
                break;
            case 'N':
                move->promotion_piece = PIECE_KNIGHT;
                break;
            default:
                panicf("parse error at character '%c'\n", c);
            }
        }
        else
        {
            // Put back the character if it wasn't a promotion
            ungetc(c, stdin);
        }
        return true;

    case 'K':
    case 'Q':
    case 'R':
    case 'B':
    case 'N':
        // Piece move: starts with piece letter
        // Patterns: Nf3, Nxe5, Ndb5, N1b5, Nd1b5
        move->piece_type = parse_piece_letter(c);

        c = getc(stdin);

        // Next could be: file (disambiguation or dest), rank (disambiguation), or 'x' (capture)
        file = parse_file(c);
        rank = parse_rank(c);

        if (file >= 0)
        {
            // Read a file - could be disambiguation or destination
            c = getc(stdin);
            int next_rank = parse_rank(c);
            int next_file = parse_file(c);

            if (next_rank >= 0)
            {
                // file + rank: could be full square (dest) or need to check further
                int saved_file = file;
                int saved_rank = next_rank;

                c = getc(stdin);
                if (c == 'x')
                {
                    // Disambiguation was file+rank, capture follows
                    move->from_file = saved_file;
                    move->from_rank = saved_rank;
                    move->is_capture = 1;
                    c = getc(stdin);
                    file = parse_file(c);
                    if (file < 0)
                        panicf("parse error at character '%c'\n", c);
                    move->to_file = file;
                    c = getc(stdin);
                    rank = parse_rank(c);
                    if (rank < 0)
                        panicf("parse error at character '%c'\n", c);
                    move->to_rank = rank;
                }
                else if (parse_file(c) >= 0)
                {
                    // Disambiguation was file+rank, destination file follows
                    move->from_file = saved_file;
                    move->from_rank = saved_rank;
                    move->to_file = parse_file(c);
                    c = getc(stdin);
                    rank = parse_rank(c);
                    if (rank < 0)
                        panicf("parse error at character '%c'\n", c);
                    move->to_rank = rank;
                }
                else
                {
                    // file + rank was the destination
                    ungetc(c, stdin);
                    move->to_file = saved_file;
                    move->to_rank = saved_rank;
                }
            }
            else if (c == 'x')
            {
                // File disambiguation followed by capture
                move->from_file = file;
                move->is_capture = 1;
                c = getc(stdin);
                file = parse_file(c);
                if (file < 0)
                    panicf("parse error at character '%c'\n", c);
                move->to_file = file;
                c = getc(stdin);
                rank = parse_rank(c);
                if (rank < 0)
                    panicf("parse error at character '%c'\n", c);
                move->to_rank = rank;
            }
            else if (next_file >= 0)
            {
                // File disambiguation followed by destination file
                move->from_file = file;
                move->to_file = next_file;
                c = getc(stdin);
                rank = parse_rank(c);
                if (rank < 0)
                    panicf("parse error at character '%c'\n", c);
                move->to_rank = rank;
            }
            else
            {
                panicf("parse error at character '%c'\n", c);
            }
        }
        else if (rank >= 0)
        {
            // Rank disambiguation
            move->from_rank = rank;
            c = getc(stdin);

            if (c == 'x')
            {
                // Rank disambiguation followed by capture
                move->is_capture = 1;
                c = getc(stdin);
            }

            file = parse_file(c);
            if (file < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_file = file;
            c = getc(stdin);
            rank = parse_rank(c);
            if (rank < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_rank = rank;
        }
        else if (c == 'x')
        {
            // Capture with no disambiguation
            move->is_capture = 1;
            c = getc(stdin);
            file = parse_file(c);
            if (file < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_file = file;
            c = getc(stdin);
            rank = parse_rank(c);
            if (rank < 0)
                panicf("parse error at character '%c'\n", c);
            move->to_rank = rank;
        }
        else
        {
            panicf("parse error at character '%c'\n", c);
        }
        return true;

    default:
        panicf("parse error at character '%c'\n", c);
    }
    return false; // Should never reach here
}
