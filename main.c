#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "parser.h"
#include "panic.h"

#define MAX_INPUT 100

/* Display welcome message and instructions */
void display_welcome(void) {
    printf("========================================\n");
    printf("   Chess Analysis Engine - APSC 143\n");
    printf("========================================\n\n");
    printf("Commands:\n");
    printf("  move <from><to> - Make a move (e.g., 'move e2e4')\n");
    printf("  print           - Display the current board\n");
    printf("  reset           - Reset the board to starting position\n");
    printf("  check           - Check if current player is in check\n");
    printf("  quit            - Exit the program\n");
    printf("  help            - Display this help message\n\n");
}

/* Display help message */
void display_help(void) {
    printf("\nMove format: Use algebraic notation (e.g., e2e4)\n");
    printf("  - Source square followed by destination square\n");
    printf("  - Columns: a-h, Rows: 1-8\n");
    printf("  - Example: 'move e2e4' moves piece from e2 to e4\n\n");
}

int main(void) {
    Board board;
    char input[MAX_INPUT];
    char command[MAX_INPUT];
    char move_str[MAX_INPUT];
    
    /* Initialize the board */
    init_board(&board);
    
    /* Display welcome message */
    display_welcome();
    print_board(&board);
    
    /* Main game loop */
    while (1) {
        printf("Turn: %s\n", (board.turn == WHITE) ? "White" : "Black");
        printf("> ");
        
        /* Get user input */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        /* Remove newline */
        input[strcspn(input, "\n")] = 0;
        
        /* Parse command */
        if (sscanf(input, "%s", command) != 1) {
            continue;
        }
        
        /* Process commands */
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            printf("Thanks for playing!\n");
            break;
        }
        else if (strcmp(command, "help") == 0) {
            display_help();
        }
        else if (strcmp(command, "print") == 0) {
            print_board(&board);
        }
        else if (strcmp(command, "reset") == 0) {
            init_board(&board);
            printf("Board reset to starting position.\n");
            print_board(&board);
        }
        else if (strcmp(command, "check") == 0) {
            if (is_in_check(&board, board.turn)) {
                printf("Yes, %s is in check!\n", 
                       (board.turn == WHITE) ? "White" : "Black");
            } else {
                printf("No, %s is not in check.\n",
                       (board.turn == WHITE) ? "White" : "Black");
            }
        }
        else if (strcmp(command, "move") == 0) {
            /* Parse move string */
            if (sscanf(input, "%*s %s", move_str) != 1) {
                printf("Invalid move format. Use: move e2e4\n");
                continue;
            }
            
            /* Parse and validate move */
            Move move;
            if (!parse_move(move_str, &move, &board)) {
                printf("Invalid move format or no piece at source.\n");
                continue;
            }
            
            /* Attempt to make the move */
            if (make_move(&board, &move)) {
                printf("Move successful: %s\n", move_str);
                print_board(&board);
                
                /* Check for check after move */
                if (is_in_check(&board, board.turn)) {
                    printf("Check! %s is in check.\n",
                           (board.turn == WHITE) ? "White" : "Black");
                }
            } else {
                printf("Illegal move. Please try again.\n");
            }
        }
        else {
            printf("Unknown command: %s\n", command);
            printf("Type 'help' for available commands.\n");
        }
    }
    
    return EXIT_SUCCESS;
}
