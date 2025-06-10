#ifndef IO_H
#define IO_H

// Type aliases for clarity and portability
typedef unsigned char  u8;  // 8-bit unsigned integer
typedef unsigned short u16; // 16-bit unsigned integer
typedef unsigned int   u32; // 32-bit unsigned integer

// =====================
// Screen Output
// =====================

/**
 * @brief Outputs a single character to the screen at the current cursor position.
 *
 * Automatically handles newline characters and wraps text at the edge
 * of the screen. If the cursor moves beyond the last screen row,
 * the terminal is scrolled up by one line.
 *
 * @param c Character to display
 */
void putc(char c);

/**
 * @brief Print a null-terminated string to the screen.
 *
 * @param s Pointer to the string to print
 */
void print(const char* s);

/**
 * @brief Clears the entire VGA text screen and resets the cursor position.
 */
void clrscr();

// =====================
// Keyboard Input
// =====================

/**
 * @brief Wait for a key press and return the corresponding ASCII character.
 * Only handles a limited US QWERTY layout.
 *
 * @return ASCII character of the pressed key
 */
char read_char();

/**
 * @brief Reads a line of input from the keyboard until ENTER is pressed
 *        or the buffer is full. Echoes typed characters to the screen.
 *
 * Input stops when a newline character ('\n') is received or when the buffer
 * reaches its maximum capacity (max_len - 1), leaving space for the null terminator.
 * In either case, a newline is printed to the screen.
 *
 * @param buf Pointer to the buffer where the input line will be stored.
 * @param max_len Maximum size of the buffer including space for the null terminator.
 */
void read_line(char* buf, int max_len);

// =====================
// Port I/O
// =====================

/**
 * @brief Read a byte from an I/O port.
 *
 * @param port The I/O port number
 * @return The byte read from the port
 */
u8 inb(u16 port);

#endif // IO_H
