#include <stdlib.h>

char* itoa(int value, char* buffer, int base){
	return lltoa((long long int)value, buffer, base);
}