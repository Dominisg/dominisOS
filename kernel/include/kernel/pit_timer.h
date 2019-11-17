#ifndef _PIT_TIMER_H
#define _PIT_TIMER_H 1
#include <stddef.h>
#include <stdint.h>
void timer_init(uint32_t frequency);
void pit_timeout_set(uint32_t val);
void pit_timeout_unset();
uint8_t pit_timeout_reached();
#endif 