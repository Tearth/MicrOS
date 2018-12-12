#include "heap.h"

heap_entry *heap;
uint32_t tail_page_index;

// TODO: This function should return 0 if there is no available memory to allocate instead of panic screen.
void *heap_alloc(uint32_t size)
{
    heap_entry *current_entry = heap;

    while (true)
    {
        if (current_entry->free)
        {
            if (current_entry->size > size + ENTRY_HEADER_SIZE)
            {
                heap_entry *next_entry = current_entry->next;

                uint32_t updated_free_size = current_entry->size - size - ENTRY_HEADER_SIZE;
                current_entry->next = (heap_entry *)((uint32_t)current_entry + size + ENTRY_HEADER_SIZE);
                current_entry->size = size;
                current_entry->free = 0;

                current_entry->next->next = next_entry;
                current_entry->next->prev = current_entry;
                current_entry->next->size = updated_free_size;
                current_entry->next->free = 1;

                return (void *)((uint32_t)current_entry + ENTRY_HEADER_SIZE);
            }
            else
            {
                if (current_entry->next == 0)
                {
                    while (size > current_entry->size + ENTRY_HEADER_SIZE)
                    {
                        tail_page_index = virtual_memory_alloc_page();
                        current_entry->size += 4 * 1024 * 1024;
                    }

                    continue;
                }
            }
        }

        current_entry = current_entry->next;
    }
}

void heap_dealloc(void *ptr)
{
    heap_entry *entry = (heap_entry *)((uint32_t)ptr - ENTRY_HEADER_SIZE);
    entry->free = 1;

    if (entry->prev != 0 && entry->prev->free)
    {
        entry->prev->size += entry->size + ENTRY_HEADER_SIZE;
        entry->prev->next = entry->next;

        if (entry->next != 0)
        {
            entry->next->prev = entry->prev;
        }

        entry = entry->prev;
    }

    if (entry->next != 0)
    {
        if (entry->next->free)
        {
            void *next_entry = (void *)((uint32_t)entry->next + ENTRY_HEADER_SIZE);
            heap_dealloc(next_entry);
        }
    }
    else
    {
        while ((float)entry->size / 1024 / 1024 > 4)
        {
            virtual_memory_dealloc_page(tail_page_index);
            entry->size -= 4 * 1024 * 1024;
            tail_page_index--;
        }
    }
}

void heap_set_base_page_index(uint32_t index)
{
    heap = (heap_entry *)(index * 1024 * 1024 * 4);
    tail_page_index = index;
}

// TODO: This shouldn't be named as "clear", set something better in the future.
void heap_clear()
{
    uint32_t allocated_virtual_pages = virtual_memory_get_allocated_pages_count();

    if (allocated_virtual_pages == 0)
    {
        heap = (heap_entry *)(virtual_memory_alloc_page() * 1024 * 1024 * 4);
    }

    heap->size = 4 * 1024 * 1024;
    heap->free = 1;
    heap->next = 0;
    heap->prev = 0;
}

void heap_dump()
{
    heap_entry *current_entry = heap;

    uint32_t index = 0;
    char buffer[16];

    while (current_entry != 0)
    {
        vga_printstring("Entry id=");
        itoa(index, buffer, 10);
        vga_printstring(buffer);
        vga_printstring(", free=");
        itoa(current_entry->free, buffer, 10);
        vga_printstring(buffer);
        vga_printstring(", size=");
        itoa(current_entry->size, buffer, 10);
        vga_printstring(buffer);
        vga_printstring("\n");

        current_entry = current_entry->next;
        index++;
    }
}