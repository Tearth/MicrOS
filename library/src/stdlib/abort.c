#include "../stdlib.h"

void abort()
{
    micros_process_exit_process(-1);
}