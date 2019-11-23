#include <kernel/term.h>
#include <kernel/tty.h>
#include <string.h>

#define COMMAND_PROMPT "COMMAND $ "
uint8_t line_count = 0;  
void shell_init(){
    terminal_writestring(COMMAND_PROMPT);
}

void shell_putchar(char character){
     if(line_count + strlen(COMMAND_PROMPT) >= 80 - 1){
         return;
     }
    terminal_putchar(character);
    line_count++;
    if(character == '\n'){
        // TODO: execute command 
        terminal_writestring(COMMAND_PROMPT);
        line_count = 0;
    }
}

void shell_backspace(){
    if(line_count > 0){
        terminal_popchar();
        line_count--;
    }
}