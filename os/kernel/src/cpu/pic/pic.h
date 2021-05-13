#ifndef PIC_H
#define PIC_H

#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA 0x21

#define SLAVE_PIC_COMMAND 0xA0
#define SLAVE_PIC_DATA 0xA1

#define PIC_READ_IRR 0x0a /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR 0x0b /* OCW3 irq service next CMD read */

#define PIC_EOI 0x20

#include <stdint.h>
#include "assembly/io.h"

//! pic_init
/*
    Initializes the 8259 Programmable Interrupt Controller driver
*/
void pic_init();

//! pic_remap
/*
    Remaps IRQ numbers, because of offset in Interrupt Descriptor Table,
    first 32 numbers are reserved for CPU exceptions
*/
void pic_remap(uint32_t master_offset, uint32_t slave_offset);

//! pic_enable_irq
/*
    Unmask interrupt in PIC.
    For now there is 2 PIC's, so we can map up to 15 hardware interrupts
    List of irq map for x86
    0 // Programmable Interrupt Timer
    1 // Keyboard
    2 // Slave PIC
    3 // COM2
    4 // COM1
    5 // LPT2
    6 // Floppy
    7 //LPT1
    8 // CMOS
    9 // Free
    10 // Free
    11 // Network interface controller
    12 // Mouse
    13 // FPU
    14 // Primary ATA Hard Disk
    15 // Secondary ATA Hard Disk
*/
void pic_enable_irq(uint8_t interrupt_number);

//! pic_enable_irq
/*
    Disable IRQ of specified number
*/
void pic_disable_irq(uint8_t interrupt_number);

//! pic_handle_irq
/*
    Function handles irq and, deals with spurious IRQs
*/
void pic_handle_irq(uint8_t interrupt_number);

//! pic_send_eoi
/*
    Tell PIC device that interrupt is handled
*/
void pic_send_eoi(uint8_t interrupt_number);

//! __pic_get_irq_reg
/*
    Returns Master and slave register values first 8 bits for master, and next for slave
*/
uint16_t __pic_get_irq_reg(uint8_t ocw3);

//! pic_get_irr
/*
    Returns the combined value of the cascaded PICs irq request register
*/
uint16_t pic_get_irr(void);

//! pic_get_isr
/*
    Returns the combined value of the cascaded PICs in-service register
*/
uint16_t pic_get_isr(void);

#endif