#include <stdio.h>
#include <kernel/pit_timer.h>
#include <kernel/tty.h>
#include <kernel/term.h>
#include <kernel/pic.h>
#include <kernel/pci.h>
#include <kernel/clock.h>
#include <kernel/ps2.h>
#include <kernel/ata.h>
#include <time.h>


void kernel_main(void) {
	terminal_init();
	timer_init(PIT_DEFAULT);
	PS2Controller_Init();
	findIDEConrollerDevice();
	ide_init(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
	printf(   
"     _                 _       _        ___  __  \n"    
"  __| | ___  _ __ ___ (_)_ __ (_)___   /___\\/ _\\ \n"   
" / _` |/ _ \\| '_ ` _ \\| | '_ \\| / __| //  //\\ \\  \n"   
"| (_| | (_) | | | | | | | | | | \\__ \\/ \\_// _\\ \\ \n"   
" \\__,_|\\___/|_| |_| |_|_|_| |_|_|___/\\___/  \\__/ \n");

	shell_init();
	while(1);
}
