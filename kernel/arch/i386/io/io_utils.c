#include <kernel/io_utils.h>

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (val));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

void io_wait(void)
{
    /* TODO: This is probably fragile. */
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}