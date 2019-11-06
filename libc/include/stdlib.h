#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);

char* lltoa(long long int value, char* buffer, int base);
char* ltoa(long int value, char* buffer, int base);
char* itoa(int value, char* buffer, int base);
int abs (int i);

#ifdef __cplusplus
}
#endif

#endif
