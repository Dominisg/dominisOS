#include <kernel/term.h>
#include <kernel/tty.h>
#include <string.h>
#include <stdio.h>
#include <kernel/drvmng.h>
#include <kernel/vfs.h>

#define COMMAND_PROMPT "COMMAND $ "
#define CMD_CNT 4

struct term_cmd{
    char cmd[16];
    void (*func)(char** params);
};

uint8_t line_count = 0;
char* params[7];

struct term_cmd cmds[] = 
{{"drvmng", drvmng}, 
 {"ls", ls},
 {"cd", cd},
 {"cat", cat}
};

static char command[80];
static char command_list[8][32];

void shell_init(){
    terminal_writestring(COMMAND_PROMPT);
}

void shell_execute(){
    uint8_t cur_param=0, cur_letter=0, printed=0;
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
    
    for(size_t i=0; i<7; i++){
        params[i] = command_list[i+1];
    }

    for(size_t i = 0; i <CMD_CNT; i++){
        if(memcmp(cmds[i].cmd, command_list[0], strlen(cmds[i].cmd)) == 0 && strlen(cmds[i].cmd) == strlen(command_list[0])){
            cmds[i].func(params);
            printed=1;
            break;
        }
    }
    if(!printed){
        printf("%s - looks strange for me\n",command_list[0]);
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

