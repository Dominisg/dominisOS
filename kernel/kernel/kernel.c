#include <stdio.h>
#include <kernel/pit_timer.h>
#include <kernel/tty.h>
#include <kernel/pic.h>

void kernel_main(void) {
	terminal_initialize();
	init_timer(50);	
	printf("\tdominisOS");
	while(1);
}
