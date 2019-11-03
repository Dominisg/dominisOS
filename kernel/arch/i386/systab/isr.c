#include <kernel/isr.h>
#include <kernel/tty.h>
#include <stdio.h>

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t* regs)
{
    printf("\nRecived interrupt:\n");
} 