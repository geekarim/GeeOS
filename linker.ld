ENTRY(start)

SECTIONS {
    . = 1M;     /* Sets the location counter (i.e., the starting address for the program) to 1 MiB (0x100000) because most Multiboot-compliant bootloaders (like GRUB) load kernels at the 1MB mark. */

    .boot :     /* GRUB needs this header within the first 8KB of the kernel file to recognize it as multiboot-compliant. */
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot_header)
    }

    .text :     /* all executable code (from .text sections in .o files) */
    {
        *(.text)
    }

    .rodata :   /* read-only data */
    {
        *(.rodata)
    }

    .data :     /* all initialized global/static variables */
    {
        *(.data)
    }

    .bss :      /* uninitialized global/static variables (e.g., char buf[128]; that’s not pre-filled). These will be zeroed out at runtime. */
    {
        *(.bss)
    }
}