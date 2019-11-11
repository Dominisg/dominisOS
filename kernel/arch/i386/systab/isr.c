#include <kernel/isr.h>
#include <kernel/tty.h>
#include <stdio.h>
#include <kernel/pic.h>
#include <kernel/io_utils.h>

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
  interrupt_handlers[n] = handler;
} 
 
static void PIC_sendEOI(uint32_t irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
 
	outb(PIC1_COMMAND,PIC_EOI);
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t* regs)
{
    printf("\nRecived interrupt: %x \n", regs->int_no);
} 
void irq_handler(registers_t* regs)
{
    PIC_sendEOI(regs->int_no - 0x20);
    if(interrupt_handlers[regs->int_no] != NULL){
        interrupt_handlers[regs->int_no](regs);
    }
}
