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
    if (c == '\n' || c == '\r' || c == EOF)
        return false;

    // Initialize all fields of the move structure to their default values.
    move->from_file = -1;
    move->from_rank = -1;
    move->to_file = -1;
    move->to_rank = -1;
    move->is_capture = 0;
    move->is_castle_kingside = 0;
    move->is_castle_queenside = 0;
    move->is_en_passant = 0;
    move->promotion_piece = PIECE_PAWN;
    move->piece_type = PIECE_PAWN;  // Default, will be overridden for pieces

    switch (c)
    {
    case 'O':
        // Castling: O-O or O-O-O
        c = getc(stdin);
        if (c != '-')
            panicf("parse error at character '%c'\n", c);
        c = getc(stdin);
        if (c != 'O')
            panicf("parse error at character '%c'\n", c);
        c = getc(stdin);
        if (c == '-')
        {
            // Check for O-O-O (queenside castle)
            c = getc(stdin);
            if (c != 'O')
                panicf("parse error at character '%c'\n", c);
            move->is_castle_queenside = 1;
            move->piece_type = PIECE_KING;
        }
        else
        {
            // O-O (kingside castle)
            move->is_castle_kingside = 1;
            move->piece_type = PIECE_KING;
            if (c != ' ' && c != '\n' && c != '\r' && c != EOF)
                ungetc(c, stdin);
        }
        // Skip trailing spaces
        while ((c = getc(stdin)) == ' ') {}
        if (c != '\n' && c != '\r' && c != EOF)
            ungetc(c, stdin);
        return true;

    case 'a': case 'b': case 'c': case 'd':
    case 'e': case 'f': case 'g': case 'h':
        // Pawn move: starts with file letter
        file = c - 'a';  // Convert 'a'=0, 'b'=1, etc.
        
        c = getc(stdin);
        if (c == 'x')
        {
            // Pawn capture: exd5
            move->from_file = file;  // Source file known
            move->is_capture = 1;
            
            // Read destination file
            c = getc(stdin);
            if (c < 'a' || c > 'h')
                panicf("parse error at character '%c'\n", c);
            file = c - 'a';
            c = getc(stdin);
        }
        // Read destination rank
        if (c < '1' || c > '8')
            panicf("parse error at character '%c'\n", c);
        rank = c - '1';  // Convert '1'=0, '2'=1, etc.
        
        move->to_file = file;
        move->to_rank = rank;
        move->piece_type = PIECE_PAWN;
        
        // Check for promotion (=Q, =R, =B, =N)
        c = getc(stdin);
        if (c == '=')
        {
            c = getc(stdin);
            if (c == 'Q')
                move->promotion_piece = PIECE_QUEEN;
            else if (c == 'R')
                move->promotion_piece = PIECE_ROOK;
            else if (c == 'B')
                move->promotion_piece = PIECE_BISHOP;
            else if (c == 'N')
                move->promotion_piece = PIECE_KNIGHT;
            else
                panicf("parse error at character '%c'\n", c);
        }
        else if (c != ' ' && c != '\n' && c != '\r' && c != EOF)
        {
            ungetc(c, stdin);
        }
        
        // Skip trailing spaces
        while ((c = getc(stdin)) == ' ') {}
        if (c != '\n' && c != '\r' && c != EOF)
            ungetc(c, stdin);
        
        return true;

    case 'K':
        move->piece_type = PIECE_KING;
        break;
    case 'Q':
        move->piece_type = PIECE_QUEEN;
        break;
    case 'R':
        move->piece_type = PIECE_ROOK;
        break;
    case 'B':
        move->piece_type = PIECE_BISHOP;
        break;
    case 'N':
        move->piece_type = PIECE_KNIGHT;
        break;

    default:
        panicf("parse error at character '%c'\n", c);
    }

    // Piece move: read optional disambiguation (file and/or rank)
    c = getc(stdin);
    if (c >= 'a' && c <= 'h')
    {
        // Could be disambiguation file or destination file
        file = c - 'a';
        c = getc(stdin);
        
        if (c >= '1' && c <= '8')
        {
            // Could be disambiguation rank or destination rank
            rank = c - '1';
            c = getc(stdin);
            
            if (c == 'x' || (c >= 'a' && c <= 'h'))
            {
                // We had disambiguation (file + rank), now read destination
                move->from_file = file;
                move->from_rank = rank;
                
                if (c == 'x')
                {
                    move->is_capture = 1;
                    c = getc(stdin);
                }
                // Read destination file
                if (c < 'a' || c > 'h')
                    panicf("parse error at character '%c'\n", c);
                file = c - 'a';
                
                // Read destination rank
                c = getc(stdin);
                if (c < '1' || c > '8')
                    panicf("parse error at character '%c'\n", c);
                rank = c - '1';
                
                move->to_file = file;
                move->to_rank = rank;
            }
            else
            {
                // Just destination (file + rank), no disambiguation
                move->to_file = file;
                move->to_rank = rank;
                if (c != ' ' && c != '\n' && c != '\r' && c != EOF)
                    ungetc(c, stdin);
            }
        }
        else if (c == 'x')
        {
            // File disambiguation followed by capture: Nfxe5
            move->from_file = file;
            move->is_capture = 1;
            
            // Read destination file
            c = getc(stdin);
            if (c < 'a' || c > 'h')
                panicf("parse error at character '%c'\n", c);
            file = c - 'a';
            
            // Read destination rank
            c = getc(stdin);
            if (c < '1' || c > '8')
                panicf("parse error at character '%c'\n", c);
            rank = c - '1';
            
            move->to_file = file;
            move->to_rank = rank;
        }
        else if (c >= 'a' && c <= 'h')
        {
            // File disambiguation followed by destination file: Ndb5
            move->from_file = file;
            file = c - 'a';
            
            // Read destination rank
            c = getc(stdin);
            if (c < '1' || c > '8')
                panicf("parse error at character '%c'\n", c);
            rank = c - '1';
            
            move->to_file = file;
            move->to_rank = rank;
        }
        else
        {
            panicf("parse error at character '%c'\n", c);
        }
    }
    else if (c >= '1' && c <= '8')
    {
        // Rank disambiguation: N1f3
        move->from_rank = c - '1';
        
        c = getc(stdin);
        if (c == 'x')
        {
            move->is_capture = 1;
            c = getc(stdin);
        }
        
        // Read destination file
        if (c < 'a' || c > 'h')
            panicf("parse error at character '%c'\n", c);
        file = c - 'a';
        
        // Read destination rank
        c = getc(stdin);
        if (c < '1' || c > '8')
            panicf("parse error at character '%c'\n", c);
        rank = c - '1';
        
        move->to_file = file;
        move->to_rank = rank;
    }
    else if (c == 'x')
    {
        // Capture without disambiguation: Nxe5
        move->is_capture = 1;
        
        // Read destination file
        c = getc(stdin);
        if (c < 'a' || c > 'h')
            panicf("parse error at character '%c'\n", c);
        file = c - 'a';
        
        // Read destination rank
        c = getc(stdin);
        if (c < '1' || c > '8')
            panicf("parse error at character '%c'\n", c);
        rank = c - '1';
        
        move->to_file = file;
        move->to_rank = rank;
    }
    else
    {
        panicf("parse error at character '%c'\n", c);
    }

    // Skip trailing spaces
    while ((c = getc(stdin)) == ' ') {}
    if (c != '\n' && c != '\r' && c != EOF)
        ungetc(c, stdin);
    
    return true;
}
