#ifndef DOMINIS_OS_IO_UTILS_H
#define DOMINIS_OS_IO_UTILS_H
#include <stdint.h>
#include <stddef.h>
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void io_wait(void);
#endif