#ifndef PROCESS_CALLS_H
#define PROCESS_CALLS_H

#include "process/manager/process_manager.h"
#include "terminal/terminal_manager.h"

void syscall_process_exit(interrupt_state *state);
void syscall_process_get_processes_count(interrupt_state *state);
void syscall_process_get_current_process_info(interrupt_state *state);
void syscall_process_get_process_info(interrupt_state *state);
void syscall_process_get_all_processes_info(interrupt_state *state);
void syscall_process_set_current_process_name(interrupt_state *state);
void syscall_process_current_process_sleep(interrupt_state *state);
void syscall_process_start_process(interrupt_state *state);
void syscall_process_set_current_process_signal_handler(interrupt_state *state);
void syscall_process_finish_signal_handler(interrupt_state *state);
void syscall_process_wait_for_process(interrupt_state *state);
void syscall_process_start_thread(interrupt_state *state);

#endif