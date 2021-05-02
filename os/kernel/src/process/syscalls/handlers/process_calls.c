#include "process_calls.h"

void syscall_process_exit(interrupt_state *state)
{
    process_manager_close_current_process(state->registers.ecx);
}

void syscall_process_get_processes_count(interrupt_state *state)
{
    state->registers.eax = process_manager_get_processes_count();
}

void syscall_process_get_current_process_info(interrupt_state *state)
{
    process_manager_get_current_process_user_info((process_user_info *)state->registers.ebx);
}

void syscall_process_get_process_info(interrupt_state *state)
{
    state->registers.eax = process_manager_get_process_user_info(state->registers.ebx, (process_user_info *)state->registers.ecx);
}

void syscall_process_get_all_processes_info(interrupt_state *state)
{
    process_manager_get_all_processes_user_info((process_user_info *)state->registers.ebx);
}

void syscall_process_set_current_process_name(interrupt_state *state)
{
    state->registers.eax = process_manager_set_current_process_name((char *)state->registers.ebx);
}

void syscall_process_current_process_sleep(interrupt_state *state)
{
    process_manager_current_process_sleep(state->registers.ebx);
}

void syscall_process_start_process(interrupt_state *state)
{
    uint32_t parent_id = (bool)state->registers.edx ? process_manager_get_current_process()->id : process_manager_get_root_process();
    uint32_t process_id = process_manager_create_process((char *)state->registers.ebx, (char *)state->registers.ecx, parent_id, (bool)state->registers.edx);
    
    terminal_struct* current_terminal = find_terminal_for_process(parent_id);
    attach_process_to_terminal(current_terminal->terminal_id, process_manager_get_process(process_id));
    
    state->registers.eax = process_id;
}

void syscall_process_set_current_process_signal_handler(interrupt_state *state)
{
    process_manager_set_current_process_signal_handler((void (*)(int))state->registers.ebx);
}

void syscall_process_finish_signal_handler(interrupt_state *state)
{
    process_manager_finish_signal_handler((signal_params *)state->registers.ebx);
}

void syscall_process_wait_for_process(interrupt_state *state)
{
    process_manager_current_process_wait_for_process(state->registers.ebx);
}

void syscall_process_start_thread(interrupt_state *state)
{
    uint32_t process_id = process_manager_get_current_process()->id;
    uint32_t thread_id = process_manager_create_thread(process_id, (void *)state->registers.ebx, (void *)state->registers.ecx);
    
    terminal_struct* current_terminal = find_terminal_for_process(process_id);
    attach_process_to_terminal(current_terminal->terminal_id, process_manager_get_process(thread_id));
    
    state->registers.eax = thread_id;
}