#include "io.h"

/**
 * @brief Compare two null-terminated strings for equality.
 *
 * @param a Pointer to the first string
 * @param b Pointer to the second string
 * @return 1 if the strings are equal, 0 otherwise
 */
int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

/**
 * @brief Interpret and execute a command string.
 * Currently supports only the "help, clear" command.
 *
 * @param cmd Pointer to the command string
 */
void run(const char* cmd) {
    if (streq(cmd, "help")) {
        print("Commands: help, clear\n");
    } else if (streq(cmd, "clear")) {
        clrscr();
    } else
        print("Unknown command\n");
}

/**
 * @brief Entry point of the kernel.
 * Initializes the shell loop and handles user input.
 */
void kernel_main() {
    clrscr();
    print("Welcome to GeeOS\n");
    
    char buf[128]; // Buffer to hold user input

    while (1) {
        print("GeeOS>");                // Display shell prompt
        read_line(buf, sizeof(buf));    // Read user input
        run(buf);                       // Execute the command
    }
}
