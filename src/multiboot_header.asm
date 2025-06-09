section .multiboot_header                                           ;This section must be present in the kernel binary so that the bootloader can recognize it as a valid Multiboot2 kernel. It must be within the first 32768 bytes of the file.
header_start:                                                       ;Label calculating the header's size and checksum.

    dd 0xe85250d6                                                   ;This magic number tells GRUB that the binary uses Multiboot2 format.

    dd 0                                                            ;Architecture `0` indicates i386 protected mode (32-bit).

    dd header_end - header_start                                    ;Header length: total size in bytes of the Multiboot header.

    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ;Multiboot2 specification requires that the sum of these three 32-bit fields equals 0 modulo 2^32. In math: (magic + architecture + header_length + checksum) % 2^32 == 0.

    dw 0                                                            ;Every Multiboot2 header must end with a tag of type `0` (called the "end tag").
    dw 0                                                            ;Flags.
    dd 8                                                            ;Size of the tag in bytes — always 8 for the end tag.
header_end:                                                         ;Label marks the end of the Multiboot header, used above to calculate header length and checksum.