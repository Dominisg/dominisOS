#include <kernel/term.h>
#include <kernel/tty.h>
#include <string.h>
#include <stdio.h>
#include <kernel/drvmng.h>
#define COMMAND_PROMPT "COMMAND $ "
uint8_t line_count = 0;

static char command[80];
static char command_list[8][32];

uint8_t* params[7];

void shell_init(){
    terminal_writestring(COMMAND_PROMPT);
}

void shell_execute(){
    uint8_t cur_param=0, cur_letter=0;
    for(int i=0; i<80; i++){
        if(command[i] == '\0')
            break;
        else if(command[i] == ' '){
            if(cur_letter>0){
                command_list[cur_param][cur_letter] = '\0';
                cur_param++;
                if(cur_param == 7)
                    break;
                cur_letter=0;
            }
        }
        else{
            command_list[cur_param][cur_letter] = command[i];
            cur_letter++;
        }
    }
    
    for(int i=0; i<7; i++){
        params[i] = command_list[i+1];
    }

    if(memcmp("drvmng", command_list[0], strlen("drvmng")) == 0){
        drvmng(params);
    }else{
        printf("%s - looks strange to me\n", command_list[0]);
    }

    memset(command_list,0,sizeof(command_list));
}

void shell_putchar(char character){
     if(line_count + strlen(COMMAND_PROMPT) >= 80 - 1){
         return;
     }
    terminal_putchar(character);
    command[line_count]=character;
    command[line_count+1]='\0';
    line_count++;
    if(character == '\n'){
        command[line_count-1]='\0';
        shell_execute();
        terminal_writestring(COMMAND_PROMPT);
        line_count = 0;
        command[line_count]='\0';
    }
}

void shell_backspace(){
    if(line_count > 0){
        terminal_popchar();
        command[line_count]='\0';
        line_count--;
    }
}

