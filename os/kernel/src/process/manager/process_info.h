#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include <stdbool.h>
#include "cpu/idt/state/interrupt_state.h"

typedef enum process_status
{
    process_status_ready,
    process_status_working,
    process_status_waiting_sleep,
    process_status_waiting_key_press,
    process_status_waiting_for_process,
} process_status;

typedef struct process_info
{
    uint32_t id;
    uint32_t parent_id;
    char name[32];
    bool is_thread;

    process_status status;
    uint32_t size_in_memory;

    void *user_stack;
    void *page_directory;
    void *heap;

    uint32_t current_cpu_usage;
    uint32_t last_cpu_usage;
    uint32_t sleep_deadline;
    uint32_t process_id_to_wait;

    interrupt_state state;
    void (*signal_handler)(int);

    uint32_t terminal_id;
} process_info;

#endif