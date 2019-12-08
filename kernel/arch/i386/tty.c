#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <kernel/io_utils.h>
#include <kernel/tty.h>
#include <kernel/vga.h>

static void update_cursor(uint8_t x, uint8_t y);

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static uint8_t terminal_scroll;
void terminal_init(void) {
	terminal_row = 1;
	terminal_column = 0;
	terminal_scroll = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_printclock(struct tm* sys_time){
	char buffer[24], *ptr;
	ptr = buffer;
	strftime(buffer, 24, "%H:%M:%S  %d-%m-%Y",sys_time);
	while(*ptr){
		terminal_buffer[29 + ptr - buffer] = vga_entry(*ptr, terminal_color);
		ptr++;
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
	unsigned char uc = c;
	if(c == '\t'){
	    for (size_t i=0; i < 4; i++){
		terminal_putchar(' ');
	    }
	    return;
	}
	if(terminal_scroll == 1){
	    	memcpy(&terminal_buffer[VGA_WIDTH], &terminal_buffer[2*VGA_WIDTH], VGA_WIDTH * (VGA_HEIGHT-1) * 2);
		for(size_t i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++){
		    terminal_buffer[i] = vga_entry(' ', terminal_color);
		}
		terminal_row--;
		terminal_scroll = 0;
	}
	if(c == '\n'){
		terminal_column = VGA_WIDTH - 1;
	}else{
		terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
	}
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_scroll = 1;
	}
	update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

void terminal_popchar(){
	if(terminal_column > 0){
		terminal_column --;
		terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
		update_cursor(terminal_column, terminal_row);
	}
}

static void update_cursor(uint8_t x, uint8_t y)
{
	uint16_t pos = y * VGA_WIDTH + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}