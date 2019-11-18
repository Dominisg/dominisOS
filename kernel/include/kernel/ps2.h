//
// Created by dominis on 15.11.19.
//
#ifndef PS2_H
#define PS2_H
#include <stddef.h>
#include <stdint.h>
uint8_t PS2Controller_Init();
uint8_t PS2Controller_Read();
void PS2Controller_Command(uint8_t comm);
void PS2Controller_Write(uint8_t data);
void PS2Controller_WriteSecondPort(uint8_t data);
#endif //PS2_H
