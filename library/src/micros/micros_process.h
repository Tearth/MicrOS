#ifndef MICROS_PROCESS_H
#define MICROS_PROCESS_H

#include <stdbool.h>
#include "../stdlib.h"
#include "micros_interrupts.h"

//! Process status retrieved from the kernel
typedef enum micros_process_status
{
    //! Process is ready to work and waits in queue
    micros_process_status_ready,
    
    //! Process is currently working
    micros_process_status_working,
    
    //! Process has called sleep function and is stopped for the specified amount of time
    micros_process_status_waiting_sleep,
    
    //! Process has called blocking keyboard function and waits for any pressed key
    micros_process_status_waiting_key_press,
    
    //! Process waits for the end of the another process (like shell is waiting for the end of the child process)
    micros_process_status_waiting_for_process
} micros_process_status;

//! Process information retrieved from the kernel
typedef struct micros_process_user_info
{
    //! Process ID
    uint32_t id;
    
    //! Parent process ID
    uint32_t parent_id;
    
    //! Process name (ended with 0)
    char name[32];

    //! Current process status
    micros_process_status status;
    
    //! Current process CPU usage calculated by the kernel for the last second
    float cpu_usage;
    
    //! Current process memory usage calculated by the kernel
    uint32_t memory_usage;
    
    //! Flag indicating if this is a process or a thread
    bool is_thread;
} __attribute__((packed)) micros_process_user_info;

//! Process registers state
typedef struct micros_signal_registers_state
{
    //! EDI register state
    uint32_t edi;
    
    //! ESI register state
    uint32_t esi;
    
    //! EBP (base pointer) register state
    uint32_t ebp;
    
    //! ESP (stack pointer) register state, contains invalid address so not really usable
    uint32_t esp_unused;
    
    //! EBX register state
    uint32_t ebx;
    
    //! EDX register state
    uint32_t edx;
    
    //! ECX register state
    uint32_t ecx;
    
    //! EAX register state
    uint32_t eax;
} micros_signal_registers_state;

//! Process FPU state
typedef struct micros_signal_fpu_state
{
    //! FPU control word
    uint32_t control_word;
    
    //! FPU status word
    uint32_t status_word;
    
    //! FPU tag word
    uint32_t tag_word;
    
    //! FPU instruction pointer offset
    uint32_t instruction_pointer_offset;
    
    //! FPU instruction pointer selector
    uint16_t instruction_pointer_selector;
    
    //! FPU opcode
    uint16_t opcode;
    
    //! FPU operand pointer offset
    uint32_t operand_pointer_offset;
    
    //! FPU operand pointer selector
    uint32_t operand_pointer_selector;
    
    //! FPU all registers state
    uint8_t registers[80];
} micros_signal_fpu_state;

//! Structure containig data passed when the signal is executed (from the point where the last instruction was executed before signal)
typedef struct micros_signal_params
{
    //! Number of the interrupt which caused the signal
    uint32_t interrupt_number;
    
    //! CR2 register state (contains address which caused page fault)
    uint32_t cr2;

    //! Saved registers state 
    micros_signal_registers_state registers;
    
    //! Saved FPU registers state
    micros_signal_fpu_state fpu_state;

    //! EIP (index pointer) register state
    uint32_t eip;
    
    //! CS (code segment) register state
    uint32_t cs;
    
    //! EFLAGS register state
    uint32_t eflags;
    
    //! ESP (stack pointer) register state
    uint32_t esp;
    
    //! SS (stack segment) register state
    uint32_t ss;
} micros_signal_params;

#ifdef __cplusplus
extern "C" {
#endif

//! Forces current process to exit
/*!
    Forces current process to exit with the specified status code (typically 0 means success).
    \param status Exit status code.
*/
void micros_process_exit_process(int status);

//! Returns number of processes
/*!
    Returns number of processes working in the operating system.
    \return Number of the working processes.
*/
uint32_t micros_process_get_processes_count();

//! Retrieves from the kernel information about the current process
/*!
    Retrieves from the kernel information about the current process and saves it to the passed structure.
    \param user_info Pointer to the structure where the information about the current process will be stored.
*/
void micros_process_get_current_process_info(micros_process_user_info *user_info);

//! Retrieves from the kernel information about the process with the specified id
/*!
    Retrieves from the kernel information about the process with the specified id and saves it to the passed structure.
    \param user_info Pointer to the structure where the information about the process with the specified id will be stored.
    \return True if process with the specified ID was found, otherwise false.
*/
bool micros_process_get_process_info(uint32_t id, micros_process_user_info *user_info);

//! Retrieves from the kernel information about all processes
/*!
    Retrieves from the kernel information about all processes id and saves them to the passed structure.
    \param user_info Pointer to the structure where the information about all processes will be stored.
*/
void micros_process_get_all_processes_info(micros_process_user_info *user_info);

//! Sets name of the current process
/*!
    Sets name of the current process. Name cannot be longer than 32 chars.
    \param name New name of the current process.
*/
void micros_process_set_current_process_name(char *name);

//! Stops process for the specified amount of time
/*!
    Stops process for the specified amount of time. Kernel won't jump to this process until the time will pass.
    \param milliseconds Milliseconds to wait.
*/
void micros_process_current_process_sleep(uint32_t milliseconds);

//! Creates new process
/*!
    Creates new process from the specified path with parameters.
    \param path Path to the executable file.
    \param arguments Arguments passed to the main function of the new process.
    \param child Flag indicates it the newly created process will be child of the current process.
    \param active Flag indicates if the newly created process will become active (on foreground).
    \return ID of the create process.
*/
uint32_t micros_process_start_process(char *path, char *arguments, bool child, bool active);

//! Sets function which will be called on the signal raise
/*!
    Sets function which will be called on the signal raise. Should be called by the standard library before entering to the main function of the program.
    \param micros_signal_params Pointer to the function.
*/
void micros_process_set_current_process_signal_handler(void (*signal_handler)(micros_signal_params*));

//! Confirms the end of signal handler execution
/*!
    Confirms the end of signal handler execution. Kernel will return to the last instruction before signal was raised.
    \param old_state Signal state retrieved from the kernel.
*/
void micros_process_finish_signal_handler(micros_signal_params *old_state);

//! Stops process until the end of the another process
/*!
    Stops process until the end of the another process with the specified ID.
    \param process_id_to_wait Process ID to wait.
*/
void micros_process_wait_for_process(uint32_t process_id_to_wait);

//! Creates new thread
/*!
    Creates new thread with the specified parameters.
    \param entry_point Address of the entry point from which thread will start.
    \param param Parameter which will be passed to the thread function.
    \return ID of the create thread.
*/
uint32_t micros_process_start_thread(void *entry_point, void *param);

//! Forces current thread to exit
/*!
    Forces current thread to exit.
*/
void micros_process_abort_thread();

#ifdef __cplusplus
}
#endif

#endif