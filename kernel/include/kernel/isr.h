#ifndef DOMINIS_OS_ISR_H
#define DOMINIS_OS_ISR_H
#include <stdint.h>
#include <stddef.h>
#include <kernel/pic.h>
typedef struct registers
{
   uint32_t ds;                  // Data segment selector
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t; 

#define IRQ0 0+PIC1
#define IRQ1 1+PIC1
#define IRQ2 2+PIC1
#define IRQ3 3+PIC1
#define IRQ4 4+PIC1
#define IRQ5 5+PIC1
#define IRQ6 6+PIC1
#define IRQ7 7+PIC1
#define IRQ8 8+PIC1
#define IRQ9 9+PIC1
#define IRQ10 10+PIC1
#define IRQ11 11+PIC1
#define IRQ12 12+PIC1
#define IRQ13 13+PIC1
#define IRQ14 14+PIC1
#define IRQ15 15+PIC1

typedef void (*isr_t)(registers_t*);
void register_interrupt_handler(uint8_t n, isr_t handler); 
#endif