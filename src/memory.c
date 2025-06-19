#include "memory.h"

// Header structure for a memory block in the heap allocator's free list
typedef struct BlockHeader {
    uint32_t size;                // Size of the allocated/freed memory block (not including this header)
    struct BlockHeader* next;    // Pointer to the next free block in the linked list
} BlockHeader;

uint8_t* page_bitmap = 0;         // Bitmap for tracking used/free pages
uint32_t bitmap_size_bytes = 0;   // Size of the bitmap in bytes
uint32_t total_pages = 0;         // Total number of physical pages
uint32_t memory_start = 0;        // Lowest physical address in usable memory
uint32_t* page_directory = 0;     // Page directory used in paging
uint32_t usable_region_count = 0; // Number of usable memory regions found (populated by parse_memory_map)

static uint32_t heap_current = KERNEL_HEAP_START; // Current end of the heap; used by kmalloc to allocate new memory blocks
static BlockHeader* free_list = NULL; // Head pointer for the free list of memory blocks (used by kmalloc/kfree)

// Array to store information about memory regions that are usable
MemoryRegion usable_memory_regions[MAX_MEMORY_REGIONS];

/**
 * @brief Initializes the physical memory allocator.
 * 
 * Calculates usable memory from the multiboot memory map, sets up a bitmap
 * to track allocated/free pages, and marks pages as free or reserved.
 * Pages used by the bitmap itself are marked as used to prevent overwrite.
 */
void init_physical_allocator() {
    memory_start = 0xFFFFFFFF;
    uint64_t memory_end = 0;

    // Determine memory bounds from usable regions
    for (uint32_t i = 0; i < usable_region_count; i++) {
        if ((uint32_t)usable_memory_regions[i].base < memory_start)
            memory_start = (uint32_t)usable_memory_regions[i].base;

        uint64_t end = usable_memory_regions[i].base + usable_memory_regions[i].length;
        if (end > memory_end)
            memory_end = end;
    }

    // Calculate total number of pages and bitmap size
    total_pages = (memory_end - memory_start) / PAGE_SIZE;
    bitmap_size_bytes = (total_pages + 7) / 8;

    // Place bitmap just after the kernel in memory, aligned to page boundary
    page_bitmap = (uint8_t*)(((uint32_t)&kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

    // Initially mark all pages as used
    for (uint32_t i = 0; i < total_pages; i++) {
        BITMAP_SET(page_bitmap, i);
    }

    // Mark usable pages as free
    for (uint32_t i = 0; i < usable_region_count; i++) {
        uint64_t start = usable_memory_regions[i].base;
        uint64_t end = start + usable_memory_regions[i].length;

        for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
            uint32_t page_index = ((uint32_t)addr - memory_start) / PAGE_SIZE;
            if (page_index < total_pages) {
                BITMAP_CLEAR(page_bitmap, page_index);
            }
        }
    }

    // Mark pages used by the bitmap itself as used
    uint32_t bitmap_start = (uint32_t)page_bitmap;
    uint32_t bitmap_end = bitmap_start + bitmap_size_bytes;

    for (uint32_t addr = bitmap_start; addr < bitmap_end; addr += PAGE_SIZE) {
        uint32_t page_index = (addr - memory_start) / PAGE_SIZE;
        if (page_index < total_pages) {
            BITMAP_SET(page_bitmap, page_index);
        }
    }
}

/**
 * @brief Allocates a single 4KB physical page.
 * 
 * @return Pointer to the start of the allocated physical page, or NULL if none available.
 */
void* alloc_page() {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!BITMAP_TEST(page_bitmap, i)) {
            BITMAP_SET(page_bitmap, i);
            return (void*)(memory_start + i * PAGE_SIZE);
        }
    }
    return 0; // Out of memory
}

/**
 * @brief Frees a previously allocated page.
 * 
 * @param addr Physical address of the page to free.
 */
void free_page(void* addr) {
    uint32_t page_index = ((uint32_t)addr - memory_start) / PAGE_SIZE;
    if (page_index < total_pages) {
        BITMAP_CLEAR(page_bitmap, page_index);
    }
}

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
void map_page_with_directory(uint32_t* pd, uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_index = vaddr >> 22;             // Top 10 bits: Page Directory index
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;    // Next 10 bits: Page Table index

    uint32_t* page_table;

    // Check if the page table exists
    if (pd[pd_index] & PAGE_PRESENT) {
        // Get the address of the existing page table
        page_table = (uint32_t*)(pd[pd_index] & ~0xFFF);
    } else {
        // Allocate a new page table
        page_table = (uint32_t*)alloc_page();
        for (int i = 0; i < 1024; i++) page_table[i] = 0;

        // Map the new page table into the page directory
        pd[pd_index] = ((uint32_t)page_table & ~0xFFF) | flags | PAGE_PRESENT;
    }

    // Map the virtual address to the physical address in the page table
    page_table[pt_index] = (paddr & ~0xFFF) | flags | PAGE_PRESENT;
}

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
void parse_memory_map(uint8_t* multiboot_info) {
    multiboot_tag* tag = (multiboot_tag*)(multiboot_info + 8); // Skips the first 8 bytes (which include total_size and reserved) to get to the first tag.

    while (tag->type != 0) { // Loop through tags until type 0 (end tag).
        if (tag->type == 6) { // Type 6 means this is the memory map tag.
            uint32_t entry_size = *((uint32_t*)((uint8_t*)tag + 8)); // Extract the size of each memory map entry.
            memory_map_entry* entry = (memory_map_entry*)((uint8_t*)tag + 16); // Set pointer to first entry.
            
            while ((uint8_t*)entry < ((uint8_t*)tag + tag->size)) { // Loop through all memory map entries in this tag.                
                // Store available memory regions
                if (entry->type == 1 && usable_region_count < MAX_MEMORY_REGIONS) {
                    usable_memory_regions[usable_region_count].base = entry->addr;
                    usable_memory_regions[usable_region_count].length = entry->len;
                    usable_region_count++;
                }

                entry = (memory_map_entry*)((uint8_t*)entry + entry_size); // Move to the next memory map entry based on size.
            }
        }

        tag = (multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7)); // Move to the next tag. Tags must be aligned to 8 bytes, so this rounds up the size.
    }
}

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
void setup_paging() {
    for (int i = 0; i < 1024; i++) page_directory[i] = 0;

    // Identity map first 4MB
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        //map_page(i, i, PAGE_WRITABLE);
        map_page_with_directory(page_directory, i, i, PAGE_WRITABLE);
    }

    // Load CR3
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging (set PG bit in CR0)
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

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
void* kmalloc(uint32_t size) {
    size = ALIGN8(size);  // Align size to 8 bytes for better memory alignment

    BlockHeader* prev = NULL;
    BlockHeader* curr = free_list;

    // Search for a suitable free block in the free list
    while (curr) {
        if (curr->size >= size) {
            // Found a block big enough
            if (prev) {
                prev->next = curr->next;  // Remove block from the middle of the list
            } else {
                free_list = curr->next;   // Remove block from the head of the list
            }

            return (void*)(curr + 1);     // Return pointer just after the header
        }

        prev = curr;
        curr = curr->next;
    }

    // No suitable free block found, allocate new memory from heap
    uint32_t total_size = sizeof(BlockHeader) + size;
    if (heap_current + total_size > KERNEL_HEAP_END) {
        return NULL;  // Out of heap memory
    }

    // Set up new block
    BlockHeader* block = (BlockHeader*)heap_current;
    block->size = size;
    heap_current += total_size;

    return (void*)(block + 1);  // Return pointer after the header
}

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
void kfree(void* ptr) {
    if (!ptr) return;  // Ignore NULL pointers

    // Retrieve the block header located just before the user pointer
    BlockHeader* block = ((BlockHeader*)ptr) - 1;

    // Insert the freed block at the front of the free list
    block->next = free_list;
    free_list = block;
}

/**
 * @brief Creates a new page directory for a user-space process.
 * 
 * Allocates and initializes a new page directory, zeroing out all entries.
 * Copies the kernel's higher-half mappings (typically entries 768–1023) from
 * the current kernel page directory to allow the user process to access kernel
 * space in a controlled manner.
 * 
 * This is typically used when setting up memory for user processes.
 *
 * @return Pointer to the newly created page directory, or NULL if allocation fails.
 */
uint32_t* create_user_page_directory() {
    // Allocate a page-aligned physical page for the new page directory
    uint32_t* new_pd = (uint32_t*)alloc_page(); 
    if (!new_pd) return NULL;

    // Initialize all 1024 entries to 0 (not present)
    for (int i = 0; i < 1024; i++) new_pd[i] = 0;

    // Copy kernel space mappings (usually the upper 1GB) into new page directory
    for (int i = 768; i < 1024; i++) {
        new_pd[i] = page_directory[i];  // Copy kernel mappings from current PD
    }

    return new_pd;
}

/**
 * @brief Maps a single user-space page into a given page directory.
 *
 * This function maps a virtual address (`vaddr`) to a physical address (`paddr`)
 * in the provided user-space page directory (`user_pd`). The mapping is created
 * with user-level and writable access permissions by combining `PAGE_USER` and `PAGE_WRITABLE` flags.
 *
 * It internally uses `map_page_with_directory()` to handle the actual page table
 * setup and mapping.
 *
 * @param user_pd  Pointer to the user's page directory where the mapping will be added.
 * @param vaddr    Virtual address to map.
 * @param paddr    Physical address to map to.
 */
void map_user_page(uint32_t* user_pd, uint32_t vaddr, uint32_t paddr) {
    // Map the given virtual address to the physical address with user and writable permissions
    map_page_with_directory(user_pd, vaddr, paddr, PAGE_USER | PAGE_WRITABLE);
}
