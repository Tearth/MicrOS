#include "../signal.h"

void (*signal_handlers[6])(int);

int raise(int sig)
{
    return raise_with_param(sig, 0);
}