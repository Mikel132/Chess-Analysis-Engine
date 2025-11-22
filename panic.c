#include "panic.h"
#include <stdio.h>
#include <stdlib.h>

/* Print error message and exit */
void panic(const char *message) {
    if (message != NULL) {
        fprintf(stderr, "ERROR: %s\n", message);
    } else {
        fprintf(stderr, "ERROR: An unknown error occurred\n");
    }
    exit(EXIT_FAILURE);
}
