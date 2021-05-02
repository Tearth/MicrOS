#include "idt.h"

volatile idt_entry idt_entries[IDT_INTERRUPT_DESCRIPTOR_TABLE_LENGTH];
volatile idt_info idt_information;
volatile interrupt_handler_definition interrupt_handlers[IDT_MAX_INTERRUPT_HANDLERS];
volatile exception_handler_definition exception_handlers[IDT_MAX_INTERRUPT_HANDLERS];

void (*process_manager_handler)(interrupt_state *state);
void (*syscalls_manager_handler)(interrupt_state *state);

exception_definition exceptions[] =
    {
        {
            .interrupt_number = 0,
            .description = "Divide-by-zero error",
        },
        {
            .interrupt_number = 1,
            .description = "Debug",
        },
        {
            .interrupt_number = 2,
            .description = "Non-maskable Interrupt",
        },
        {
            .interrupt_number = 3,
            .description = "Breakpoint",
        },
        {
            .interrupt_number = 4,
            .description = "Overflow",
        },
        {
            .interrupt_number = 5,
            .description = "Bound Range Exceeded",
        },
        {
            .interrupt_number = 6,
            .description = "Invalid Opcode",
        },
        {
            .interrupt_number = 7,
            .description = "Device Not Available",
        },
        {
            .interrupt_number = 8,
            .description = "Double Fault",
        },
        {
            .interrupt_number = 9,
            .description = "Coprocessor Segment Overrun",
        },
        {
            .interrupt_number = 10,
            .description = "Invalid TSS",
        },
        {
            .interrupt_number = 11,
            .description = "Segment Not Present",
        },
        {
            .interrupt_number = 12,
            .description = "Stack-Segment Fault",
        },
        {
            .interrupt_number = 13,
            .description = "General Protection Fault",
        },
        {
            .interrupt_number = 14,
            .description = "Page Fault",
        },
        {
            .interrupt_number = 15,
            .description = "Reserved",
        },
        {
            .interrupt_number = 16,
            .description = "x87 Floating-Point Exception",
        },
        {
            .interrupt_number = 17,
            .description = "Alignment Check",
        },
        {
            .interrupt_number = 18,
            .description = "Machine Check",
        },
        {
            .interrupt_number = 19,
            .description = "SIMD Floating-Point Exception",
        },
        {
            .interrupt_number = 20,
            .description = "Virtualization Exception",
        },
        {
            .interrupt_number = 21,
            .description = "Reserved",
        },
        {
            .interrupt_number = 22,
            .description = "Reserved",
        },
        {
            .interrupt_number = 23,
            .description = "Reserved",
        },
        {
            .interrupt_number = 24,
            .description = "Reserved",
        },
        {
            .interrupt_number = 25,
            .description = "Reserved",
        },
        {
            .interrupt_number = 26,
            .description = "Reserved",
        },
        {
            .interrupt_number = 27,
            .description = "Reserved",
        },
        {
            .interrupt_number = 28,
            .description = "Reserved",
        },
        {
            .interrupt_number = 29,
            .description = "Reserved",
        },
        {
            .interrupt_number = 30,
            .description = "Security Exception",
        },
        {
            .interrupt_number = 31,
            .description = "Reserved",
        }};

void idt_init()
{
    // Init Interrupt Descriptor Table descriptor
    idt_information.size = sizeof(idt_entry) * IDT_INTERRUPT_DESCRIPTOR_TABLE_LENGTH - 1;
    idt_information.offset = (uint32_t)&idt_entries;

    // Exceptions
    idt_set(0, idt_exc0, false);
    idt_set(1, idt_exc1, false);
    idt_set(2, idt_exc2, false);
    idt_set(3, idt_exc3, false);
    idt_set(4, idt_exc4, false);
    idt_set(5, idt_exc5, false);
    idt_set(6, idt_exc6, false);
    idt_set(7, idt_exc7, false);
    idt_set(8, idt_exc8, false);
    idt_set(9, idt_exc9, false);
    idt_set(10, idt_exc10, false);
    idt_set(11, idt_exc11, false);
    idt_set(12, idt_exc12, false);
    idt_set(13, idt_exc13, false);
    idt_set(14, idt_exc14, false);
    idt_set(15, idt_exc15, false);
    idt_set(16, idt_exc16, false);
    idt_set(17, idt_exc17, false);
    idt_set(18, idt_exc18, false);
    idt_set(19, idt_exc19, false);
    idt_set(20, idt_exc20, false);
    idt_set(21, idt_exc21, false);
    idt_set(22, idt_exc22, false);
    idt_set(23, idt_exc23, false);
    idt_set(24, idt_exc24, false);
    idt_set(25, idt_exc25, false);
    idt_set(26, idt_exc26, false);
    idt_set(27, idt_exc27, false);
    idt_set(28, idt_exc28, false);
    idt_set(29, idt_exc29, false);
    idt_set(30, idt_exc30, false);
    idt_set(31, idt_exc31, false);

    // Hardware interrupts (IRQ)
    idt_set(32, idt_int32, false); // Programmable Interrupt Timer
    idt_set(33, idt_int33, false); // Keyboard
    idt_set(34, idt_int34, false); // Cascade
    idt_set(35, idt_int35, false); // COM2
    idt_set(36, idt_int36, false); // COM1
    idt_set(37, idt_int37, false); // LPT2
    idt_set(38, idt_int38, false); // Floppy
    idt_set(39, idt_int39, false); // LPT1
    idt_set(40, idt_int40, false); // CMOS
    idt_set(41, idt_int41, false); // Free
    idt_set(42, idt_int42, false); // Free
    idt_set(43, idt_int43, false); // NIC
    idt_set(44, idt_int44, false); // Mouse
    idt_set(45, idt_int45, false); // FPU
    idt_set(46, idt_int46, false); // Primary ATA Hard Disk
    idt_set(47, idt_int47, false); // Secondary ATA Hard Disk

    // Software interrupts
    idt_set(50, idt_int50, true);

    // Load Interrupt Descriptor Table to the register
    __asm__("lidt %0" ::"m"(idt_information));

    // Add system calls interrupt handler
    idt_attach_interrupt_handler(18, idt_syscalls_interrupt_handler);
}

void idt_set(uint8_t index, uint32_t (*handler)(interrupt_state *state), bool user_interrupt)
{
    uint32_t handler_address = (uint32_t)handler;

    idt_entries[index].offset_0_15 = handler_address & 0xFFFF;
    idt_entries[index].offset_16_31 = (handler_address >> 16) & 0xFFFF;
    idt_entries[index].present = 1;
    idt_entries[index].selector = 8;
    idt_entries[index].privilege_level = user_interrupt ? 3 : 1;
    idt_entries[index].type = Trap_32Bit;
}

void idt_unset(uint8_t index)
{
    idt_entries[index].present = 0;
}

void idt_attach_interrupt_handler(uint8_t interrupt_number, bool (*handler)(interrupt_state *state))
{
    for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
    {
        if (interrupt_handlers[i].handler == 0)
        {
            interrupt_handlers[i].interrupt_number = interrupt_number + 32;
            interrupt_handlers[i].handler = handler;
            break;
        }
    }
}

void idt_detach_interrupt_handler(uint8_t interrupt_number, bool (*handler)(interrupt_state *state))
{
    for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
    {
        if (interrupt_handlers[i].interrupt_number == interrupt_number + 32 && interrupt_handlers[i].handler == handler)
        {
            interrupt_handlers[i].handler = 0;
            break;
        }
    }
}

void idt_attach_exception_handler(uint8_t exception_number, void (*handler)(exception_state *state))
{
    for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
    {
        if (exception_handlers[i].handler == 0)
        {
            exception_handlers[i].exception_number = exception_number;
            exception_handlers[i].handler = handler;
            break;
        }
    }
}

void idt_detach_exception_handler(uint8_t exception_number, void (*handler)(exception_state *state))
{
    for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
    {
        if (exception_handlers[i].exception_number == exception_number && exception_handlers[i].handler == handler)
        {
            exception_handlers[i].handler = 0;
            break;
        }
    }
}

void idt_attach_process_manager(void (*handler)(interrupt_state *state))
{
    process_manager_handler = handler;
}

void idt_attach_syscalls_manager(void (*handler)(interrupt_state *state))
{
    syscalls_manager_handler = handler;
}

void idt_global_int_handler(interrupt_state *state)
{
	//Note that we have offset in idt
    pic_handle_irq(state->interrupt_number - 32);

    for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
    {
        if (interrupt_handlers[i].interrupt_number == state->interrupt_number && interrupt_handlers[i].handler != 0)
        {
            if (interrupt_handlers[i].handler(state))
                break;
        }
    }

    io_disable_interrupts();
    pic_send_eoi(state->interrupt_number - 32);

    if (process_manager_handler != 0)
        process_manager_handler(state);
}

void idt_global_exc_handler(exception_state *state)
{
    bool allow_exception_in_kernel = state->interrupt_number == 1 || state->interrupt_number == 3;
    if (allow_exception_in_kernel || state->cs == 27)
    {
        for (int i = 0; i < IDT_MAX_INTERRUPT_HANDLERS; i++)
        {
            if (exception_handlers[i].exception_number == state->interrupt_number && exception_handlers[i].handler != 0)
            {
                exception_handlers[i].handler(state);
            }
        }
    }

    if (allow_exception_in_kernel)
    {
        return;
    }

    for (int i = 0; i < 32; i++)
    {
        if (exceptions[i].interrupt_number == state->interrupt_number)
        {
            char exception_string[64];
            char error_code_msg[] = ". Error code: ";
            char error_code_str[10];

            memset(exception_string, 0, 64);
            memset(error_code_str, 0, 10);

            uint32_t description_length = strlen(exceptions[i].description);
            uint32_t error_code_msg_length = strlen(error_code_msg);

            memcpy(exception_string, exceptions[i].description, description_length);
            memcpy(exception_string + description_length, error_code_msg, error_code_msg_length);

            itoa(state->error_code, error_code_str, 16);
            memcpy(exception_string + description_length + error_code_msg_length, error_code_str, 10);

            panic_screen_show(state, state->interrupt_number, exception_string);
            break;
        }
    }
}

bool idt_syscalls_interrupt_handler(interrupt_state *state)
{
    syscalls_manager_handler(state);
    return false;
}