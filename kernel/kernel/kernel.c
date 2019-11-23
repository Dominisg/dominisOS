#include <stdio.h>
#include <kernel/pit_timer.h>
#include <kernel/tty.h>
#include <kernel/term.h>
#include <kernel/pic.h>
#include <kernel/clock.h>
#include <kernel/ps2.h>
#include <time.h>


void kernel_main(void) {
	terminal_init();
	timer_init(50);
	PS2Controller_Init();
	printf(   
"     _                 _       _        ___  __  \n"    
"  __| | ___  _ __ ___ (_)_ __ (_)___   /___\\/ _\\ \n"   
" / _` |/ _ \\| '_ ` _ \\| | '_ \\| / __| //  //\\ \\  \n"   
"| (_| | (_) | | | | | | | | | | \\__ \\/ \\_// _\\ \\ \n"   
" \\__,_|\\___/|_| |_| |_|_|_| |_|_|___/\\___/  \\__/ \n");

	shell_init();
	while(1);
}
