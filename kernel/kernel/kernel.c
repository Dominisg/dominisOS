#include <stdio.h>
#include <kernel/pit_timer.h>
#include <kernel/tty.h>
#include <kernel/pic.h>
#include <kernel/clock.h>
#include <kernel/acpi.h>
#include <time.h>


void kernel_main(void) {
	terminal_initialize();
	init_timer(50);	
	printf(   
"     _                 _       _        ___  __  \n"    
"  __| | ___  _ __ ___ (_)_ __ (_)___   /___\\/ _\\ \n"   
" / _` |/ _ \\| '_ ` _ \\| | '_ \\| / __| //  //\\ \\  \n"   
"| (_| | (_) | | | | | | | | | | \\__ \\/ \\_// _\\ \\ \n"   
" \\__,_|\\___/|_| |_| |_|_|_| |_|_|___/\\___/  \\__/ \n");

	if(getFADT()){
		printf("Mamy FADT!\n");
	}

	while(1);
}
