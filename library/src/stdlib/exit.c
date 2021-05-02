#include "../stdlib.h"

void exit(int status)
{
    micros_process_exit_process(status);
}