#include <ctype.h>

int isdigit(int arg){
    if(arg >= 48 && arg <= 57 )
        return 1;
    return 0;
}