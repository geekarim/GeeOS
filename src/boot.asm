global start            ;Exposes the `start` label as the entry point to the linker, so the bootloader or GRUB knows where execution begins.
extern kernel_main      ;Tells the assembler that `kernel_main` is defined in another file, and will be linked later.

section .text           ;Marks the beginning of the code section.
bits 32                 ;Assembles this file in 32-bit mode.
start:                  ;The actual entry point after GRUB hands control to the kernel.

    mov esp, stack_top  ;Sets up the stack by putting the address of `stack_top` into `ESP`, the stack pointer. This is required because C code (like `kernel_main()`) depends on a working stack for function calls, variables, etc.

    push ebx            ;Push multiboot info pointer as argument to the C function kernel_main.
    call kernel_main    ;Calls the C function kernel_main, which is the real start of the OS logic. After this, the CPU executes the C code.

    cli                 ;Clear interrupts — disables them to avoid unexpected behavior after the kernel returns.
.hang:                  ;Label marking an infinite halt loop — ensures safety after kernel ends.
    hlt                 ;Halt the CPU (wait for the next interrupt — which never comes here).
    jmp .hang           ;Infinite loop to stop execution if `kernel_main` ever returns (it shouldn't).

section .bss            ;Reserves uninitialized memory space.

align 16                ;Ensures stack_bottom is 16-byte aligned — good for performance and ABI (Application Binary Interface) compliance.
stack_bottom:           ;Label marking the end of 16 KiB reserved space.
    resb 16384          ;Reserve 16 KiB (16 × 1024 = 16384 bytes) for the stack. Can be increased if the kernel gets more complex.
stack_top:              ;Label pointing to the top of the stack. Since x86 stacks grow downward, this is where the stack starts.
