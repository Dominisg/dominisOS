#ifndef _PIC_H
#define _PIC_H 1
#include <stddef.h>
#include <stdint.h>

#define PIC_EOI		0x20		/* End-of-interrupt command code */
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

void PIC_remap(int32_t offset1, int32_t offset2);
void IRQ_set_mask(uint8_t IRQline);
void IRQ_clear_mask(uint8_t IRQline);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);
#endif 