//
// Created by dominis on 02.11.19.
//
#ifndef ACPI_H
#define ACPI_H
#include <stddef.h>
#include <stdint.h>
/*
RSDP (Root System Description Pointer) is a data structure
used in the ACPI programming interface. 
*/
struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
 struct RSDPDescriptor firstPart;
 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));


uint32_t findRSDPDescriptor(void);
#endif //ACPI_H
