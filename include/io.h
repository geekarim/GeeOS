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
 * @brief Print a single character to the screen.
 * Handles newline wrapping automatically.
 *
 * @param c Character to print
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
 * @brief Read a line of text from the keyboard into the provided buffer.
 * Terminates on ENTER and echoes typed characters to the screen.
 *
 * @param buf Pointer to a character buffer (must be large enough)
 */
void read_line(char* buf);

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
