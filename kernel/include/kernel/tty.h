#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <time.h>

void terminal_init(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_printclock(struct tm* sys_time);
void terminal_clear();
void terminal_popchar();
void terminal_setcolor(uint8_t);

#endif
