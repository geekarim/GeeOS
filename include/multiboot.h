#ifndef MULTIBOOT_H
#define MULTIBOOT_H

// Represents a generic tag in the Multiboot2 info structure
typedef struct {
    uint32_t type;  // identifies the kind of tag (e.g., memory map = 6).
    uint32_t size;  // total size of the tag including the header.
} multiboot_tag;

// Describes one memory region in the memory map
typedef struct {
    uint64_t addr;      // Start of region
    uint64_t len;       // Length of region
    uint32_t type;      // 1 = available RAM, other = reserved
    uint32_t reserved;  // Unused
} memory_map_entry;

// Structure representing the Multiboot information passed by GRUB to the kernel
typedef struct multiboot_info {
    uint32_t flags;         // Bitfield indicating which parts of the struct are valid
    
    // Memory layout (available if flags[0] is set)
    uint32_t mem_lower;      // Lower memory in kilobytes (typically up to 640 KB)
    uint32_t mem_upper;      // Upper memory in kilobytes (above 1 MB, e.g. 512 MB = 524288)

    // Boot device info (available if flags[1] is set)
    uint32_t boot_device;    // BIOS device the OS image was loaded from

    // Kernel command-line string (available if flags[2] is set)
    uint32_t cmdline;        // Physical address of the command-line string

    // Boot module info (available if flags[3] is set)
    uint32_t mods_count;     // Number of modules loaded
    uint32_t mods_addr;      // Physical address of the first module structure

    // ELF section header table (available if flags[5] is set)
    struct {
        uint32_t num;        // Number of entries
        uint32_t size;       // Size of each entry
        uint32_t addr;       // Address of the section header table
        uint32_t shndx;      // Index of the string table section
    } elf_sec;

    // Memory map (available if flags[6] is set)
    uint32_t mmap_length;    // Total size of the memory map
    uint32_t mmap_addr;      // Physical address of the memory map

    // Drive info (available if flags[7] is set)
    uint32_t drives_length;  // Size of the drive structures
    uint32_t drives_addr;    // Address of the drive structures

    // ROM configuration table (available if flags[8] is set)
    uint32_t config_table;   // Physical address of the ROM configuration table

    // Boot loader name (available if flags[9] is set)
    uint32_t boot_loader_name; // Address of a string with the bootloader name

    // APM table (available if flags[10] is set)
    uint32_t apm_table;      // Address of the APM BIOS info table

    // VBE (VESA BIOS Extensions) information (available if flags[11] is set)
    uint32_t vbe_control_info;   // Address of VBE control information
    uint32_t vbe_mode_info;      // Address of VBE mode information
    uint16_t vbe_mode;           // Current VBE mode
    uint16_t vbe_interface_seg;  // VBE interface segment
    uint16_t vbe_interface_off;  // VBE interface offset
    uint16_t vbe_interface_len;  // VBE interface length
} __attribute__((packed)) multiboot_info_t;

// `__attribute__((packed))` tells the compiler to not add padding between fields, 
// ensuring the structure layout matches what GRUB provides exactly.

#endif // MULTIBOOT_H