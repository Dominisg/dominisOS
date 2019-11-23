#include <kernel/keyboard.h>
#include <kernel/ps2.h>
#include <kernel/isr.h>
#include <kernel/term.h>
#include <stdio.h>

#define KB_STATE_KEY_RELEASED 0x1
#define KB_STATE_SHIFT_PRESSED 0x2
uint8_t state = 0;

enum KB_KEYS{
    KB_F1 = 128, KB_F2, KB_F3, KB_F4, KB_F5, KB_F6, KB_F7, KB_F8, KB_F9,
    KB_F10, KB_F11, KB_F12, KB_LALT, KB_LSHIFT, KB_LCTRL, KB_CAPSLOCK,
    KB_RSHIFT, KB_BACKSPACE, KB_ESC, KB_NUMLOCK, KB_SCROLLLOCK  
};
enum SCAN_CODES{
    KB_KEY_RELEASE = 0xF0,
    KB_KEY_SPECIAL = 0xE0,
};

uint8_t keypressed_map[128] ={0, KB_F9, 0, KB_F5, KB_F3, KB_F1, KB_F2, KB_F12, 0, KB_F10,
/*10*/ KB_F8, KB_F6, KB_F4, 0x09, '`', 0, 0, KB_LALT,KB_LSHIFT, 0,
/*20*/ KB_LCTRL, 'q', '1', 0,0,0,'z','s','a','w',
/*30*/ '2', 0, 0, 'c', 'x', 'd', 'e', '4', '3', 0,
/*40*/ 0, ' ', 'v', 'f', 't', 'r', '5', 0, 0, 'n',
/*50*/ 'b', 'h', 'g', 'y', '6', 0, 0, 0 ,'m', 'j',
/*60*/ 'u', '7', '8', 0, 0, ',', 'k', 'i', 'o', '0',
/*70*/ '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
/*80*/ 0, 0, '\'', 0, '[', '=', 0, 0,KB_CAPSLOCK, KB_RSHIFT, 
/*90*/ '\n', ']', 0, '\\', 0, 0, 0, 0, 0, 0,
/*100*/0, 0, KB_BACKSPACE, 0, 0, '1', 0, '4', '7', 0,
/*110*/ 0, 0, '0', '.', '2', '5', '6', '8', KB_ESC, KB_NUMLOCK,
/*120*/KB_F11, '+', '3', '-', '*', '9', KB_SCROLLLOCK
};

uint8_t _123456789_shiftmap[10]={
    '!','@','#','$','%','^','&','*','('
};

uint8_t _key_value_shiftmap[24]={
        '0', ')',
        '`','~',
        ',','<',
        '.','>',
        '/', '?',
        ';', ':',
        '\'','"',
        '[', '{',
        ']', '}',
        '-', '_',
        '=', '+',
        '\\', '|',
};

void keyboard_callback(){
    uint8_t character;
    uint8_t code = PS2Controller_Read();

    if(state & KB_STATE_KEY_RELEASED){
        state &= ~KB_STATE_KEY_RELEASED;
        if(code < 128 && ((keypressed_map[code] == KB_RSHIFT) ||
            (keypressed_map[code] == KB_LSHIFT)) ){
                state &= ~KB_STATE_SHIFT_PRESSED;   
            }
        return;
    }

    if(code < 128 && code > 0){
        character = (char)keypressed_map[code];
        switch(character){
        case KB_RSHIFT:
        case KB_LSHIFT:
            state |= KB_STATE_SHIFT_PRESSED;
            return;
        case KB_BACKSPACE:
            shell_backspace();
            return;
        default:
            if(state & KB_STATE_SHIFT_PRESSED && code < 100){
                if(character <= 'z' && character >= 'a' ){
                    character = character -'a' + 'A';
                }
                else if(character <= '9' && character >='1'){
                    character = _123456789_shiftmap[character-'1'];
                }
                else{
                    for(int i=0; i<12; i++){
                        if(_key_value_shiftmap[2 * i] == character){
                            character = _key_value_shiftmap[2 * i + 1];
                        }
                    }
                }
                
            }if(character < 128){
                shell_putchar(character);
            }
            break;
        }
        
    }else{
        switch(code){
            case KB_KEY_RELEASE:
            state |= KB_STATE_KEY_RELEASED;
            return;
        }
    }
}

void init_keyboard(uint8_t interrupt){
    register_interrupt_handler(interrupt, &keyboard_callback);
}