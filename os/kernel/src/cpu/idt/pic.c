#include "pic.h"

//Count
uint16_t spurious_count = 0;

void pic_init()
{
	// Move all interrupts from 0..7 to 32..39
	// First parameter means that IRQ0 data will have 0x20 (32) offset in IDT
	// Second parameter means that IRQ8 data will have 0x28 (40) offset in IDT
	pic_remap(0x20, 0x28);
}

void pic_remap(uint32_t master_offset, uint32_t slave_offset)
{
	// Send initialization sequence to the master PIC and slave PIC
	io_out_byte(MASTER_PIC_COMMAND, 0x11);
	io_out_byte(SLAVE_PIC_COMMAND, 0x11);

	// Set master PIC offset
	io_out_byte(MASTER_PIC_DATA, master_offset);

	// Set slave PIC offset
	io_out_byte(SLAVE_PIC_DATA, slave_offset);

	// Tell master PIC that there is a slave PIC on the IRQ2 (4 = 0100)
	io_out_byte(MASTER_PIC_DATA, 4);

	// Tell slave PIC about his role
	io_out_byte(SLAVE_PIC_DATA, 2);

	// We are in 8086 mode
	// for master there is init bit (0) and buffer bit (3)
	io_out_byte(MASTER_PIC_DATA, 0x05);
	// for slave there is only init bit(0)
	io_out_byte(SLAVE_PIC_DATA, 0x01);

	// Mask all IRQ's
	io_out_byte(MASTER_PIC_DATA, 0xF);
	io_out_byte(SLAVE_PIC_DATA, 0xF);

	// Enable interrupts
	io_enable_interrupts();
}

void pic_enable_irq(uint8_t interrupt_number)
{
	//If interrupt number correpsonds to master
	if (interrupt_number < 8)
	{
		uint8_t mask = io_in_byte(MASTER_PIC_DATA);
		mask &= ~(1 << interrupt_number);
		io_out_byte(MASTER_PIC_DATA, mask);
		//exit function
		return;
	}

	//If interrupt number corresponds to slave
	//take care of unmasking slave in master
	if (interrupt_number > 7 && interrupt_number < 16)
	{
		uint8_t mask = io_in_byte(SLAVE_PIC_DATA);
		interrupt_number -= 8;
		mask &= ~(1 << interrupt_number);
		io_out_byte(SLAVE_PIC_DATA, mask);

		//unmask slave in master
		mask = io_in_byte(MASTER_PIC_DATA);
		mask &= ~(1 << 2);
		io_out_byte(MASTER_PIC_DATA, mask);
		return;
	}
}

void pic_disable_irq(uint8_t interrupt_number)
{
	//If interrupt number correpsonds to master
	if (interrupt_number < 8)
	{
		uint8_t mask = io_in_byte(MASTER_PIC_DATA);
		mask |= (1 << interrupt_number);
		io_out_byte(MASTER_PIC_DATA, mask);
		//exit function
		return;
	}

	//If interrupt number corresponds to slave
	//take care of unmasking slave in master
	if (interrupt_number > 7 && interrupt_number < 16)
	{
		uint8_t mask = io_in_byte(SLAVE_PIC_DATA);
		interrupt_number -= 8;
		mask |= (1 << interrupt_number);
		io_out_byte(SLAVE_PIC_DATA, mask);
		return;
	}
}

void pic_handle_irq(uint8_t interrupt_number)
{
	//Spurious IRQ
	if ((interrupt_number + 1) % 8)
	{
		//For master
		if (interrupt_number == 7)
		{
			//pic_get_isr returns 16 wide value, we need only 8th bit:
			//if the bit is set 1, the IRQ is true, but if not it's spurious
			if (!(pic_get_isr() && 0x80))
			{
				spurious_count++;
				return;
			}
		}

		//For slave
		if (interrupt_number == 15)
		{
			//pic_get_isr returns 16 wide value, we need only 16th bit:
			//if the bit is set 1, the IRQ is true, but if not it's spurious
			if (!(pic_get_isr() && 0x8000))
			{
				//Send EOI ONLY to master
				io_out_byte(MASTER_PIC_COMMAND, PIC_EOI);
				spurious_count++;
				return;
			}
		}
	}
}

void pic_send_eoi(uint8_t interrupt_number)
{
	if (interrupt_number >= 8)
		io_out_byte(SLAVE_PIC_COMMAND, PIC_EOI);

	io_out_byte(MASTER_PIC_COMMAND, PIC_EOI);
}

uint16_t __pic_get_irq_reg(uint8_t ocw3)
{
	io_out_byte(MASTER_PIC_COMMAND, ocw3);
	io_out_byte(SLAVE_PIC_COMMAND, ocw3);
	return (io_in_byte(SLAVE_PIC_COMMAND) << 8) | io_in_byte(MASTER_PIC_COMMAND);
}

uint16_t pic_get_irr(void)
{
	return __pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(void)
{
	return __pic_get_irq_reg(PIC_READ_ISR);
}