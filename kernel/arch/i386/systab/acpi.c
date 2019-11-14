#include <kernel/acpi.h>
#include <string.h>
#include <kernel/tty.h>

struct RSDPDescriptor20 *rsdp = NULL;

uint32_t findRSDPDescriptor(){
    if(rsdp != NULL){
        return (uint32_t) rsdp;
    }
   
   uint16_t *edba_ptr_location = (uint16_t*) 0x40E;
   char *edba = (char*)(uint32_t) *edba_ptr_location;
   char *ptr = edba;

    // Unsure if it's correct. Needing checking.
    for (uint32_t i = 0; i < 1024; i++){
        if(memcmp(ptr, "RSD PTR ", 8) == 0 ){
            rsdp = (struct RSDPDescriptor20 *) ptr;
            printf("RSDPtr : %x", (uint32_t) rsdp);
            return (uint32_t) rsdp;
        }
        ptr++;
    }
    ptr = (char*)0xE0000;
    while(ptr != (char*)0xFFFFF){
        if(memcmp(ptr, "RSD PTR ", 8) == 0){
            rsdp = (struct RSDPDescriptor20 *) ptr;
            printf("RSDPtr : %x", (uint32_t) rsdp);
            return (uint32_t) rsdp;
        }
        ptr++;
    }

    return 0;
}

