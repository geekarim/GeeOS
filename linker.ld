ENTRY(start)

SECTIONS {
    . = 1M;                     /* Sets the location counter (i.e., the starting address for the program) to 1 MiB (0x100000) because most Multiboot-compliant bootloaders (like GRUB) load kernels at the 1MB mark. */

    .boot :                 /* GRUB needs this header within the first 8KB of the kernel file to recognize it as multiboot-compliant. */
    {
        /* ensure that the multiboot header is at the beginning */
        *(.multiboot_header)    /* Include all .multiboot_header sections from all object files (*) into this final .multiboot_header section. */
    } :boot

    .text :                 /* all executable code (from .text sections in .o files) */
    {
        *(.text)                /* Include all .text sections from all object files (*) into this final .text section. */
    } :text

    .rodata :               /* read-only data */
    {
        *(.rodata)              /* Include all .rodata sections from all object files (*) into this final .rodata section. */
    } :rodata

    .data :                 /* all initialized global/static variables */
    {
        *(.data)                /* Include all .data sections from all object files (*) into this final .data section. */
    } :data

    .bss :                  /* uninitialized global/static variables (e.g., char buf[128]; that’s not pre-filled). These will be zeroed out at runtime. */
    {
        *(.bss)                 /* Include all .bss sections from all object files (*) into this final .bss section. */
        . = ALIGN(4096);    /* This line adjusts the location counter (i.e., the memory offset) so that the next memory section starts on a 4096-byte (4 KiB) boundary—a common page size in x86 memory management. */
    } :bss

    kernel_end = .;         /* Marks the end of the kernel binary in memory, used to place data (e.g., page bitmap) right after it. */
}

PHDRS   /* Program headers */
{
    boot    PT_LOAD FLAGS(0);          /* Multiboot header - no RWX flags */
    text    PT_LOAD FLAGS(5);          /* Read + Execute */
    rodata  PT_LOAD FLAGS(4);          /* Read only */
    data    PT_LOAD FLAGS(6);          /* Read + Write */
    bss     PT_LOAD FLAGS(6);          /* Read + Write */
}