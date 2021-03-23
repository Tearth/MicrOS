#include "../signal.h"

void (*signal_handlers[6])(int);

void __signal_init()
{
    signal_handlers[0] = default_sigabrt_handler;
    signal_handlers[1] = default_sigfpe_handler;
    signal_handlers[2] = default_sigill_handler;
    signal_handlers[3] = default_sigint_handler;
    signal_handlers[4] = default_sigsegv_handler;
    signal_handlers[5] = default_sigterm_handler;
}