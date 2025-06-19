#include "io.h"

// VGA (Video Graphics Array) text buffer base address
static uint16_t* vga = (uint16_t*) 0xB8000;

// Default text color: light cyan (0xB) on black (0x0)
static uint8_t color = 0xB;

// Current cursor position
static uint32_t row = 0, col = 0;

static int shift_pressed = 0;
static int caps_lock_on = 0;

/**
 * @brief Scrolls the VGA text screen up by one line.
 *
 * Copies each row of text one line up in the VGA buffer,
 * clears the last row, and adjusts the cursor position to stay
 * within screen bounds. Called automatically when text reaches
 * the bottom of the screen.
 */
static void scroll() {
    // Move each row up one
    for (uint32_t r = 1; r < 25; r++) {
        for (uint32_t c = 0; c < 80; c++) {
            vga[(r - 1) * 80 + c] = vga[r * 80 + c];
        }
    }

    // Clear the last row
    for (uint32_t c = 0; c < 80; c++) {
        vga[(24 * 80) + c] = (color << 8) | ' ';
    }

    // Update cursor position
    if (row > 0) row--;
}

/**
 * @brief Outputs a single character to the screen at the current cursor position.
 *
 * Automatically handles newline characters and wraps text at the edge
 * of the screen. If the cursor moves beyond the last screen row,
 * the terminal is scrolled up by one line.
 *
 * @param c Character to display.
 */
void putc(char c) {
    if (c == '\n') {
        col = 0;
        row++;
    } else {
        vga[row * 80 + col++] = (color << 8) | c;
        if (col == 80) {
            col = 0;
            row++;
        }
    }
    if (row == 25) {
        scroll();
    }
}

/**
 * @brief Outputs a null-terminated string to the screen.
 *
 * @param s Pointer to the string to print.
 */
void print(const char* s) {
    while (*s) putc(*s++);
}

/**
 * @brief Reads a single byte from the given I/O port.
 *
 * @param port I/O port to read from.
 * @return Byte read from the port.
 */
uint8_t inb(uint16_t port) {
    uint8_t r;
    __asm__ volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

/**
 * @brief Waits for and returns the next ASCII character input from the keyboard.
 *
 * Processes basic US QWERTY layout, including Shift and Caps Lock functionality for
 * letter case and symbol selection. Ignores key releases and unmapped keys.
 *
 * Special handling includes:
 * - Shift keys (left/right) for uppercase letters and symbols.
 * - Caps Lock toggle for letter case.
 * - Ignores non-character scancodes and key release events.
 *
 * @return The ASCII character corresponding to the pressed key.
 */
char read_char() {

    // US QWERTY mapping
    static const char keymap[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0, 'a','s','d','f','g',
        'h','j','k','l',';', '\'', '`', 0, '\\', 'z','x','c','v','b','n','m',',', '.',
        '/', 0, '*', 0, ' ', 0
    };

    static const char shift_keymap[128] = {
        0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b','\t',
        'Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
        'A','S','D','F','G','H','J','K','L',':','"','~', 0, '|',
        'Z','X','C','V','B','N','M','<','>','?', 0, '*', 0, ' ', 0
    };
    
    while (1) {
        // Wait for keyboard buffer to have data
        if ((inb(0x64) & 1) == 0) continue;

        uint8_t code = inb(0x60);

        // Handle Shift press (0x2A = left shift, 0x36 = right shift)
        if (code == 0x2A || code == 0x36) {
            shift_pressed = 1;
            continue;
        }

        // Handle Shift release (0xAA = left, 0xB6 = right)
        if (code == 0xAA || code == 0xB6) {
            shift_pressed = 0;
            continue;
        }

        // Handle Caps Lock (toggle on 0x3A)
        if (code == 0x3A) {
            caps_lock_on = !caps_lock_on;
            continue;
        }

        // Ignore key releases (high bit set)
        if (code & 0x80) continue;

        char c = keymap[code]; // Always start from base map

        // If it's a letter, apply Caps Lock and Shift logic
        if (c >= 'a' && c <= 'z') {
            if (caps_lock_on ^ shift_pressed) {
                c = c - ('a' - 'A'); // Convert to uppercase
            }
        } else {
            // For non-letter keys (like 1 -> !), use shift_keymap if Shift is pressed
            if (shift_pressed) {
                c = shift_keymap[code];
            }
        }

        return c;
    }
}

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
void read_line(char* buf, int max_len) {
    int i = 0;

    while (1) {
        char c = read_char();

        if (c == '\n' || i == max_len - 1) {
            putc('\n');
            break;
        }

        buf[i++] = c;
        putc(c);
    }

    buf[i] = '\0'; // Null-terminate the input string
}

/**
 * @brief Clears the entire VGA text screen and resets the cursor.
 */
void clrscr() {
    for (uint32_t r = 0; r < 25; r++) {
        for (uint32_t c = 0; c < 80; c++) {
            vga[r * 80 + c] = (color << 8) | ' ';
        }
    }
    row = 0;
    col = 0;
}
