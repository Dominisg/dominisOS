#include <stdlib.h>
char* ltoa(long int value, char* buffer, int base){
	return lltoa((long long int)value, buffer, base);
}