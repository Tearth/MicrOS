#ifndef MEMORY_CALLS_H
#define MEMORY_CALLS_H

#include "../../IDT/interrupt_state.h"
#include "../../../Memory/Manager/Heap/heap.h"

void syscall_heap_alloc_memory(interrupt_state *state);
void syscall_heap_realloc_memory(interrupt_state *state);
void syscall_heap_dealloc_memory(interrupt_state *state);
void syscall_heap_get_object_size(interrupt_state *state);
void syscall_heap_verify_integrity(interrupt_state *state);
void syscall_heap_get_process_heap(interrupt_state *state);

#endif