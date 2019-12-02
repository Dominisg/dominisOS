//
// Created by dominis on 30.11.19.
//
#ifndef ATA_H
#define ATA_H
#include <stddef.h>
#include <stdint.h>
void ide_initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3, uint32_t BAR4);
#endif //ATA_H
