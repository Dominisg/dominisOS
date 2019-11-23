#ifndef _KERNEL_TERM_H
#define _KERNEL_TERM_H
#include <stddef.h>
#include <stdint.h>

void shell_init();
void shell_putchar(char character);
void shell_backspace();
#endif
