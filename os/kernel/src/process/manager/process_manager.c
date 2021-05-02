#include "process_manager.h"

kvector processes;
volatile uint32_t current_process_id = 0;
volatile uint32_t next_process_id = 1;
volatile uint32_t last_task_switch = 0;
volatile uint32_t last_cpu_recalculation = 0;
volatile uint32_t root_process_id = 0;
volatile bool run_scheduler_on_next_interrupt = false;

extern void enter_user_space(interrupt_state *address);

void process_manager_init()
{
    last_task_switch = timer_get_system_clock();
    last_cpu_recalculation = timer_get_system_clock();

    kvector_init(&processes);
    idt_attach_process_manager(process_manager_interrupt_handler);
    idt_attach_interrupt_handler(1, process_manager_keyboard_interrupt_handler);
}

uint32_t process_manager_create_process(char *path, char *parameters, uint32_t parent_id, bool active)
{
    uint32_t path_length = strlen(path) + 1;
    uint32_t parameters_length = strlen(parameters) + 1;

    void *path_in_kernel_heap = heap_kernel_alloc(path_length, 0);
    void *parameters_in_kernel_heap = heap_kernel_alloc(parameters_length, 0);
    heap_entry *old_heap = heap_get_user_heap();

    memcpy(path_in_kernel_heap, path, path_length);
    memcpy(parameters_in_kernel_heap, parameters, parameters_length);

    process_info *process = (process_info *)heap_kernel_alloc(sizeof(process_info), 0);
    process->id = next_process_id++;
    process->parent_id = parent_id;
    process->status = process_status_ready;
    process->current_cpu_usage = 0;
    process->last_cpu_usage = 0;
    process->sleep_deadline = 0;
    process->is_thread = false;

    process->page_directory = heap_kernel_alloc(1024 * 4, 1024 * 4);

    paging_table_entry *page_directory = paging_get_page_directory();
    memcpy(process->page_directory, (void *)paging_get_kernel_page_directory(), 1024 * 4);

    paging_set_page_directory(process->page_directory);

    filesystem_file_info process_file_info;
    filesystem_get_file_info(path_in_kernel_heap, &process_file_info);
    
    uint8_t *process_content = heap_kernel_alloc(process_file_info.size, 0);
    filesystem_read_file(path_in_kernel_heap, process_content, 0, process_file_info.size);

    elf_header *app_header = elf_get_header(process_content);
    
    bool error = false;
    if(app_header->magic_number != 0x7F)
    {
        error = true;
        goto release;
    }
    
    uint32_t initial_page = elf_loader_load(process_content);
    process->size_in_memory = elf_get_total_size_in_memory(process_content);

    uint32_t stack_align = 4 - (process->size_in_memory % 4);
    process->user_stack = (void *)(initial_page * 1024 * 1024 * 4 + process->size_in_memory + stack_align) + 1024 * 1024;
    process->heap = (void *)((uint32_t)process->user_stack) + 4;

    heap_set_user_heap((void *)(process->heap));
    heap_init_user_heap();

    void *path_in_user_heap = heap_user_alloc(path_length, 0);
    void *parameters_in_user_heap = heap_user_alloc(parameters_length, 0);

    memcpy(path_in_user_heap, path_in_kernel_heap, path_length);
    memcpy(parameters_in_user_heap, parameters_in_kernel_heap, parameters_length);

    *((uint32_t *)process->user_stack - 0) = (uint32_t)parameters_in_user_heap;
    *((uint32_t *)process->user_stack - 1) = (uint32_t)path_in_user_heap;

    process->state.eip = app_header->entry_position;
    process->state.esp = (uint32_t)process->user_stack - 8;
    process->state.interrupt_number = 0;
    process->state.eflags = 0x200;
    process->state.cs = 0x1B;
    process->state.ss = 0x23;

    process->state.registers.eax = 0;
    process->state.registers.ebx = 0;
    process->state.registers.ecx = 0;
    process->state.registers.edx = 0;
    process->state.registers.esp_unused = (uint32_t)process->state.esp;
    process->state.registers.ebp = 0;
    process->state.registers.esi = 0;
    process->state.registers.edi = 0;

    process->state.fpu_state.control_word = 0x37F;
    process->state.fpu_state.status_word = 0;
    process->state.fpu_state.tag_word = 0xFFFF;
    process->state.fpu_state.instruction_pointer_offset = 0;
    process->state.fpu_state.instruction_pointer_selector = 0;
    process->state.fpu_state.opcode = 0;
    process->state.fpu_state.operand_pointer_offset = 0;
    process->state.fpu_state.operand_pointer_selector = 0;

    if (processes.count == 0)
    {
        current_process_id = process->id;
    }

    kvector_add(&processes, process);
  
release:
    heap_kernel_dealloc(path_in_kernel_heap);
    heap_kernel_dealloc(parameters_in_kernel_heap);
    heap_kernel_dealloc(process_content);

    paging_set_page_directory(page_directory);
    heap_set_user_heap(old_heap);
    
    if(error)
    {
        return -1;
    }

    return process->id;
}

uint32_t process_manager_create_thread(uint32_t process_id, void *entry_point, void *stack)
{
    process_info *process = process_manager_get_process_info(process_id);
    
    process_info *thread = (process_info *)heap_kernel_alloc(sizeof(process_info), 0);
    thread->id = next_process_id++;
    thread->parent_id = process_id;
    thread->status = process_status_ready;
    thread->current_cpu_usage = 0;
    thread->last_cpu_usage = 0;
    thread->sleep_deadline = 0;
    thread->page_directory = process->page_directory;
    thread->user_stack = stack;
    thread->heap = process->heap;
    thread->signal_handler = process->signal_handler;
    thread->terminal_id = process->terminal_id;
    thread->is_thread = true;

    thread->state.eip = (uint32_t)entry_point;
    thread->state.esp = (uint32_t)thread->user_stack - 2 * sizeof(void*);
    thread->state.interrupt_number = 0;
    thread->state.eflags = 0x200;
    thread->state.cs = 0x1B;
    thread->state.ss = 0x23;

    thread->state.registers.eax = 0;
    thread->state.registers.ebx = 0;
    thread->state.registers.ecx = 0;
    thread->state.registers.edx = 0;
    thread->state.registers.esp_unused = (uint32_t)thread->state.esp;
    thread->state.registers.ebp = 0;
    thread->state.registers.esi = 0;
    thread->state.registers.edi = 0;

    thread->state.fpu_state.control_word = 0x37F;
    thread->state.fpu_state.status_word = 0;
    thread->state.fpu_state.tag_word = 0xFFFF;
    thread->state.fpu_state.instruction_pointer_offset = 0;
    thread->state.fpu_state.instruction_pointer_selector = 0;
    thread->state.fpu_state.opcode = 0;
    thread->state.fpu_state.operand_pointer_offset = 0;
    thread->state.fpu_state.operand_pointer_selector = 0;
    memcpy(thread->name, "SLAVE", 32);

    if (processes.count == 0)
    {
        current_process_id = thread->id;
    }

    kvector_add(&processes, thread);
    return thread->id;
}

process_info *process_manager_get_process(uint32_t process_id)
{
    for (uint32_t i = 0; i < processes.count; i++)
    {
        if (((process_info *)processes.data[i])->id == process_id)
        {
            return (process_info *)processes.data[i];
        }
    }

    return NULL;
}

process_info *process_manager_get_current_process()
{
    return processes.data[current_process_id];
}

void process_manager_set_root_process(uint32_t process_id)
{
    root_process_id = process_id;
}

uint32_t process_manager_get_root_process()
{
    return root_process_id;
}

void process_manager_save_current_process_state(interrupt_state *state, uint32_t delta)
{
    process_info *old_process = processes.data[current_process_id];
    memcpy(&old_process->state, state, sizeof(interrupt_state));

    old_process->current_cpu_usage += delta;
    
    if (old_process->is_thread)
    {
        process_info *parent_process = process_manager_get_process_info(old_process->parent_id);
        parent_process->current_cpu_usage += delta;
    }
}

void process_manager_switch_to_next_process()
{
    uint32_t new_process_id = current_process_id;
    process_info *new_process = NULL;

    while (1)
    {
        new_process_id = (new_process_id + 1) % processes.count;
        new_process = processes.data[new_process_id];

        if (new_process->status == process_status_waiting_sleep)
        {
            if (timer_get_system_clock() >= new_process->sleep_deadline)
            {
                new_process->status = process_status_ready;
                break;
            }
        }

        if (new_process->status == process_status_ready)
        {
            break;
        }

        if (new_process_id == current_process_id)
        {
            if (new_process->status == process_status_working)
            {
                break;
            }

            io_enable_interrupts();
            __asm__ volatile("hlt");
            io_disable_interrupts();

            last_task_switch = timer_get_system_clock();
        }
    }

    process_info *old_process = processes.data[current_process_id];

    current_process_id = new_process_id;
    if (old_process->status == process_status_working)
    {
        old_process->status = process_status_ready;
    }
    new_process->status = process_status_working;

    paging_set_page_directory(new_process->page_directory);
    heap_set_user_heap((void *)(new_process->heap));

    enter_user_space(&new_process->state);
}

void process_manager_close_current_process(bool is_thread)
{
    process_info *current_process = processes.data[current_process_id];
    process_manager_close_process(current_process->id, is_thread, true);
}

void process_manager_close_process(uint32_t process_id, bool is_thread, bool allow_to_switch)
{
    io_disable_interrupts();
    
    uint32_t process_index = process_manager_get_process_index(process_id);
    process_info* process = processes.data[process_index];
    kvector_remove(&processes, process_index);
    
    dettached_process_from_terminal(process);
    
    if (!is_thread)
    {
        void *page_directory_backup = (uint32_t*)paging_get_page_directory();
        void *heap_backup = (uint32_t*)heap_get_user_heap();
        
        paging_set_page_directory(process->page_directory);
        heap_set_user_heap((void *)(process->heap));
        
        uint32_t allocated_pages_count = virtual_memory_get_allocated_pages_count(false);
        for(uint32_t i=0; i<allocated_pages_count; i++)
        {
            virtual_memory_dealloc_last_page(false);
        }
        
        paging_set_page_directory(page_directory_backup);
        heap_set_user_heap(heap_backup);
    }
    
    for(int i = processes.count - 1; i >= 0; i--)
    {
        process_info* potential_child_process = processes.data[i];
        if(potential_child_process->status == process_status_waiting_for_process && potential_child_process->process_id_to_wait == process->id)
        {
            potential_child_process->status = process_status_ready;
        }
        
        if(potential_child_process->parent_id == process->id)
        {
            process_manager_close_process(potential_child_process->id, potential_child_process->is_thread, false);
        }
    }
    
    io_enable_interrupts();

    if (allow_to_switch)
    {
        if (processes.count > 0)
        {
            bool switch_to_next_process = process_index == current_process_id;
            
            if(process->id <= current_process_id)
            {
                current_process_id--;
            }
            
            if(switch_to_next_process)
            {
                process_manager_switch_to_next_process();
            }
        }
        else
        {
            panic_screen_show(NULL, 31, "No active process");
        }
    }
}

uint32_t process_manager_get_processes_count()
{
    return processes.count;
}

uint32_t process_manager_get_process_index(uint32_t process_id)
{
    for (uint32_t i = 0; i < processes.count; i++)
    {
        process_info *process = processes.data[i];
        if (process->id == process_id)
        {
            return i;
        }
    }
    
    return 0;
}

process_info *process_manager_get_process_info(uint32_t id)
{
    for (uint32_t i = 0; i < processes.count; i++)
    {
        process_info *process = processes.data[i];
        if (process->id == id)
        {
            return process;
        }
    }

    return NULL;
}

void process_manager_get_current_process_user_info(process_user_info *user_info)
{
    process_manager_get_process_user_info(current_process_id, user_info);
}

bool process_manager_get_process_user_info(uint32_t id, process_user_info *user_info)
{
    process_info *process = process_manager_get_process_info(id);
    if (process == NULL)
    {
        return false;
    }

    process_manager_convert_process_info_to_user_info(process, user_info);
    return true;
}

void process_manager_get_all_processes_user_info(process_user_info *user_info)
{
    for (uint32_t i = 0; i < processes.count; i++)
    {
        process_info *process = processes.data[i];
        process_manager_convert_process_info_to_user_info(process, &user_info[i]);
    }
}

bool process_manager_set_current_process_name(char *name)
{ 
    process_info *process = processes.data[current_process_id];
    if (process != NULL)
    {
        memcpy(process->name, name, 32);
        return true;
    }

    return false;
}

bool process_manager_set_current_process_signal_handler(void (*signal_handler)(int))
{
    process_info *process = processes.data[current_process_id];
    if (process != NULL)
    {
        process->signal_handler = signal_handler;
        return true;
    }

    return false;
}

void process_manager_finish_signal_handler(signal_params *old_state)
{
    interrupt_state state;
    state.cs = old_state->cs;
    state.ss = old_state->ss;
    state.eip = old_state->eip;
    state.esp = old_state->esp;
    state.eflags = old_state->eflags;

    memcpy(&state.registers, &old_state->registers, sizeof(registers_state));
    memcpy(&state.fpu_state, &old_state->fpu_state, sizeof(fpu_state));

    enter_user_space(&state);
}

void process_manager_current_process_sleep(uint32_t milliseconds)
{
    process_info *current_process = processes.data[current_process_id];
    
    process_info *process = process_manager_get_process_info(current_process->id);
    process->status = process_status_waiting_sleep;
    process->sleep_deadline = timer_get_system_clock() + milliseconds;

    run_scheduler_on_next_interrupt = true;
}

void process_manager_current_process_wait_for_key_press()
{
    process_info *current_process = processes.data[current_process_id];
    
    process_info *process = process_manager_get_process_info(current_process->id);
    process->status = process_status_waiting_key_press;

    run_scheduler_on_next_interrupt = true;
}

void process_manager_current_process_wait_for_process(uint32_t process_id_to_wait)
{
    if(process_manager_get_process_info(process_id_to_wait) != NULL)
    {
        process_info *process = processes.data[current_process_id];
        process->status = process_status_waiting_for_process;
        process->process_id_to_wait = process_id_to_wait;
        
        run_scheduler_on_next_interrupt = true;
    }
}

void process_manager_convert_process_info_to_user_info(process_info *process, process_user_info *user_info)
{
    user_info->id = process->id;
    user_info->parent_id = process->parent_id;
    memcpy(user_info->name, process->name, 32);

    user_info->status = (uint32_t)process->status;
    user_info->cpu_usage = process->last_cpu_usage;
    user_info->memory_usage = process_manager_get_process_memory_usage(process);
    user_info->is_thread = process->is_thread;
}

uint32_t process_manager_get_process_memory_usage(process_info *process)
{
    void *old_page_directory = paging_get_page_directory();
    void *old_heap = heap_get_user_heap();

    paging_set_page_directory(process->page_directory);
    heap_set_user_heap((void *)process->heap);
    uint32_t allocated_pages = virtual_memory_get_allocated_pages_count(false);

    heap_set_user_heap(old_heap);
    paging_set_page_directory(old_page_directory);

    return allocated_pages * 4;
}

void process_manager_interrupt_handler(interrupt_state *state)
{
    uint32_t current_time = timer_get_system_clock();
    uint32_t delta = current_time - last_task_switch;

    if (last_cpu_recalculation + 1000 <= current_time)
    {
        for (uint32_t i = 0; i < processes.count; i++)
        {
            process_info *process = processes.data[i];
            process->last_cpu_usage = process->current_cpu_usage;
            process->current_cpu_usage = 0;
        }

        last_cpu_recalculation = current_time;
    }

    if (run_scheduler_on_next_interrupt || (state->cs == 0x1B && delta >= 10))
    {
        run_scheduler_on_next_interrupt = false;
        last_task_switch = timer_get_system_clock();

        if (state->cs == 0x1B)
        {
            process_manager_save_current_process_state(state, delta);
        }

        process_manager_switch_to_next_process(state);
    }
}

bool process_manager_keyboard_interrupt_handler(interrupt_state *state)
{
    for (uint32_t i = 0; i < processes.count; i++)
    {
        process_info *process = processes.data[i];
        terminal_struct* terminal = find_terminal_for_process(process->id);
        uint32_t active_terminal_id = terminal_manager_get_active_terminal_id();
        
        if (terminal->terminal_id == active_terminal_id && process->status == process_status_waiting_key_press)
        {
            process->status = process_status_ready;
        }
    }
    
    return false;
}

void process_manager_run()
{
    run_scheduler_on_next_interrupt = true;
}