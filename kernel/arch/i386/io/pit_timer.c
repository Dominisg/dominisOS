#include <kernel/pit_timer.h>
#include <kernel/isr.h>
#include <stdio.h>
#include <kernel/pic.h>
#include <kernel/io_utils.h>
#include <kernel/tty.h>
#include <kernel/clock.h>

uint32_t tick = 0;
static uint32_t pit_timeout_value = 0;
static uint32_t sleep_count = 0;

uint8_t pit_timeout_reached(){
    return pit_timeout_value > 0 ? 0 : 1;
}

void pit_timeout_set(uint32_t val){
    pit_timeout_value = val;
}

void pit_timeout_unset(){
    pit_timeout_value = 0;
}

static void timer_callback(registers_t* regs)
{
    struct tm* sys_time;
	sys_time = (struct tm*)read_rtc();
    terminal_printclock(sys_time);
    
    if(pit_timeout_value){
        pit_timeout_value--;
    }
    if(sleep_count){
        sleep_count--;
    }
}

void timer_init(uint32_t frequency)
{
   // Firstly, register our timer callback.
   register_interrupt_handler(IRQ0, &timer_callback);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32_t divisor = 1193180 / frequency;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
} 

void sleep(uint32_t ms){
    while(sleep_count){
        
    };
}