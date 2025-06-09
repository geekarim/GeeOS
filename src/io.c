#include "io.h"

// VGA (Video Graphics Array) text buffer base address
static u16* vga = (u16*) 0xB8000;

// Default text color: light cyan (0xB) on black (0x0)
static u8 color = 0xB;

// Current cursor position
static u32 row = 0, col = 0;

/**
 * @brief Outputs a single character to the screen at the current cursor position.
 * Automatically handles newlines and line wrapping.
 *
 * @param c Character to display
 */
void putc(char c) {
    if (c == '\n') {
        col = 0;
        if (++row == 25) row = 0; // Wrap vertically
    } else {
        vga[row * 80 + col++] = (color << 8) | c;
        if (col == 80) {
            col = 0;
            if (++row == 25) row = 0; // Wrap vertically
        }
    }
}

/**
 * @brief Outputs a null-terminated string to the screen.
 *
 * @param s Pointer to the string to print
 */
void print(const char* s) {
    while (*s) putc(*s++);
}

/**
 * @brief Reads a single byte from the given I/O port.
 *
 * @param port I/O port to read from
 * @return Byte read from the port
 */
u8 inb(u16 port) {
    u8 r;
    __asm__ volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

/**
 * @brief Blocks until a keyboard key is pressed, then returns the mapped ASCII character.
 *
 * @return Pressed character (if mapped), otherwise skips unrecognized codes
 */
char read_char() {
    while (1) {
        // Wait for keyboard buffer to have data
        if ((inb(0x64) & 1) == 0) continue;

        u8 code = inb(0x60);

        // Simple US QWERTY mapping for demonstration (keys 2 to 13)
        static const char keymap[128] = {
            0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
            'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0, 'a','s','d','f','g',
            'h','j','k','l',';', '\'', '`', 0, '\\', 'z','x','c','v','b','n','m',',', '.',
            '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };

        if (code < sizeof(keymap))
            return keymap[code];
    }
}

/**
 * @brief Reads a line of input from the keyboard until ENTER is pressed.
 * The input is echoed to the screen and stored in the provided buffer.
 *
 * @param buf Pointer to buffer where input line is stored
 */
void read_line(char* buf) {
    int i = 0;

    while (1) {
        char c = read_char();

        if (c == '\n') {
            putc('\n');
            break;
        }

        buf[i++] = c;
        putc(c);
    }

    buf[i] = 0; // Null-terminate the input string
}

/**
 * @brief Clears the entire VGA text screen and resets the cursor.
 */
void clrscr() {
    for (u32 r = 0; r < 25; r++) {
        for (u32 c = 0; c < 80; c++) {
            vga[r * 80 + c] = (color << 8) | ' ';
        }
    }
    row = 0;
    col = 0;
}
