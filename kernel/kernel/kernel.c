#include <stdio.h>

#include <kernel/tty.h>

void kernel_main(void) {
	terminal_initialize();
	asm volatile ("int $0x01");
	printf("\tdominisOS");
}
