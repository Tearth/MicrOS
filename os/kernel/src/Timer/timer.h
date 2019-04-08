#ifndef TIMER_H
#define TIMER_H

#define CHANNEL0_PORT 0x40
#define MODE_COMMAND_REGISTER 0x43

#define TIMER_FREQUENCY_HZ 1193182
#define TARGET_FREQUENCY 1000

#include "Interrupts/PIC/pic.h"
#include "Interrupts/IDT/idt.h"
#include "Assembly/io.h"

void timer_init();
uint16_t timer_get_reload_value();
uint32_t timer_get_expected_clock_fraction();
uint32_t timer_get_system_clock();
void timer_interrupt();

#endif