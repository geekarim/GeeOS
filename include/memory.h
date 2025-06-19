#ifndef MEMORY_H
#define MEMORY_H

#ifndef NULL
#define NULL ((void*)0)
#endif

#include "stdint.h"
#include "multiboot.h"

#define PAGE_SIZE 4096 // Size of a memory page in bytes (4 KB)
#define MAX_MEMORY_REGIONS 32 // Maximum number of usable memory regions the system can track
#define PAGE_PRESENT 0x1 // Page table entry flag: page is present in memory
#define PAGE_WRITABLE 0x2 // Page table entry flag: page is writable
#define PAGE_USER 0x4 // Page table entry flag: page is accessible from user mode

// Kernel heap configuration
#define KERNEL_HEAP_START  ((uint32_t)&kernel_end)  // Heap starts after kernel
#define KERNEL_HEAP_SIZE   0x100000                 // 1 MB heap
#define KERNEL_HEAP_END    (KERNEL_HEAP_START + KERNEL_HEAP_SIZE)  // Heap limit

// Bitmap utility macros
#define BITMAP_SET(bitmap, bit)    (bitmap[(bit)/8] |=  (1 << ((bit)%8)))
#define BITMAP_CLEAR(bitmap, bit)  (bitmap[(bit)/8] &= ~(1 << ((bit)%8)))
#define BITMAP_TEST(bitmap, bit)   (bitmap[(bit)/8] &   (1 << ((bit)%8)))

// Macro to align memory size to the nearest 8-byte boundary for alignment safety
#define ALIGN8(x) (((x) + 7) & ~7)

// Memory region structure used to represent a usable block of physical memory.
typedef struct {
    uint64_t base;   // Start address of the memory region
    uint64_t length; // Length of the memory region in bytes
} MemoryRegion;

extern char kernel_end; // Symbol defined by linker indicating end of kernel binary

//extern MemoryRegion usable_memory_regions[MAX_MEMORY_REGIONS]; // Detected usable memory regions
//extern uint32_t usable_region_count; // Count of usable memory regions
//extern uint32_t bitmap_size_bytes; // Size of the bitmap in bytes
//extern uint32_t total_pages; // Total number of physical pages
//extern uint32_t memory_start; // Lowest physical address in usable memory
//extern uint32_t* page_directory; // Page directory used in paging

/**
 * @brief Initializes the physical memory allocator.
 * 
 * Calculates usable memory from the multiboot memory map, sets up a bitmap
 * to track allocated/free pages, and marks pages as free or reserved.
 * Pages used by the bitmap itself are marked as used to prevent overwrite.
 */
void init_physical_allocator();

/**
 * @brief Allocates a single 4KB physical page.
 * 
 * @return Pointer to the start of the allocated physical page, or NULL if none available.
 */
void* alloc_page();

/**
 * @brief Frees a previously allocated page.
 * 
 * @param addr Physical address of the page to free.
 */
void free_page(void* addr);

/**
 * @brief Parses the Multiboot memory map from the provided Multiboot information structure.
 *
 * This function iterates through the Multiboot tags starting from the
 * given multiboot_info pointer, locates the memory map tag (type 6),
 * and extracts usable memory regions from the memory map entries.
 *
 * @param multiboot_info Pointer to the start of the Multiboot information structure.
 *                       The first 8 bytes (total_size and reserved) are skipped,
 *                       and parsing starts from the first tag.
 *
 * This function populates the global usable_memory_regions array and increments
 * usable_region_count for each available memory region found.
 */
void parse_memory_map(uint8_t* multiboot_info);

/**
 * @brief Sets up basic paging for the system by initializing the page directory,
 * identity mapping the first 4MB of physical memory with writable pages,
 * loading the page directory base register (CR3), and enabling paging by
 * setting the PG bit in the CR0 control register.
 *
 * This function assumes that the page_directory pointer is already
 * allocated and points to a zeroed 4KB-aligned page directory.
 * The identity mapping maps virtual addresses 0x00000000 to 0x003FFFFF
 * directly to the same physical addresses, which is typically required
 * during early kernel initialization.
 */
void setup_paging();

/**
 * @brief Maps a virtual address to a physical address in a given page directory.
 *
 * This function ensures that the specified virtual address is mapped to the
 * given physical address within the provided page directory. If the corresponding
 * page table does not exist, it will be dynamically allocated and initialized.
 *
 * @param pd     Pointer to the page directory.
 * @param vaddr  Virtual address to map.
 * @param paddr  Physical address to map to.
 * @param flags  Page flags (e.g., PAGE_WRITABLE, PAGE_USER).
 */
void map_page_with_directory(uint32_t* pd, uint32_t vaddr, uint32_t paddr, uint32_t flags);

/**
 * @brief Allocates a block of memory from the kernel heap.
 *
 * This function implements a simple heap allocator using a free list of previously
 * freed blocks. It first attempts to find a suitable free block from the free list.
 * If none are large enough, it allocates a new block from the heap.
 *
 * The memory returned is aligned to 8 bytes. Metadata (BlockHeader) is stored just
 * before the returned memory pointer.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL if out of memory.
 */
void* kmalloc(uint32_t size);

/**
 * @brief Frees a previously allocated block of memory.
 * 
 * Adds the block pointed to by ptr back to the free list for reuse in future allocations.
 * The pointer must have been returned by a prior call to kmalloc().
 * 
 * This function does not merge adjacent free blocks (no coalescing).
 *
 * @param ptr Pointer to the memory block to free. If NULL, the function does nothing.
 */
void kfree(void* ptr);

#endif // MEMORY_H