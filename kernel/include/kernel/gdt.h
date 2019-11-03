//
// Created by dominis on 02.11.19.
//
/*
    Defines a GDT descriptor.
    Compiler attribute 'packed' prevent
    alignment in the structure
*/
#ifndef DOMINIS_OS_GDT_H
#define DOMINIS_OS_GDT_H
#include <stddef.h>
#include <stdint.h>
struct gdt_entry_struct
{
   uint16_t limit_low;           // The lower 16 bits of the limit.
   uint16_t base_low;            // The lower 16 bits of the base.
   uint8_t  base_middle;         // The next 8 bits of the base.
   uint8_t  access;              // Access flags, determine what ring this segment can be used in.
   uint8_t  granularity;
   uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct 
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;

void init_gdt();

#endif //DOMINIS_OS_GDT_H
