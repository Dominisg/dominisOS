#include <kernel/pci.h>
#include <kernel/io_utils.h>
#include <stdio.h>

uint16_t pciConfigReadWord (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint32_t pciConfigReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
        uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;
 
    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    tmp = inl(0xCFC);
    return (tmp);
}

uint16_t checkifIDE(uint8_t bus, uint8_t slot, uint8_t* func) {
    uint16_t vendor, class, header_type, progIF;

    if ((vendor = pciConfigReadWord(bus,slot,0,0)) != 0xFFFF) {
       class = pciConfigReadWord(bus,slot,0,0x0A);
       //printf("%x\n", class);
       if(class == 0x0101){
            progIF = pciConfigReadWord(bus,slot,0,0x08) >> 8;
            if(progIF == 0x80 || progIF == 0x8A){
                *func = 0;
                return (vendor);
            }
       }


       header_type= pciConfigReadWord(bus,slot,0,0x0F) & 0xFF;
       if( header_type == 0x80){
        /* It is a multi-function device, so check remaining functions */
            for(int function = 1; function < 8; function++) {
                if((vendor = pciConfigReadWord(bus,slot,function,0)) != 0xFFFF) {
                    class = pciConfigReadWord(bus,slot,function,0x0A);
                    if(class == 0x0101){
                        progIF = pciConfigReadWord(bus,slot,function,0x08) >> 8;
                        if(progIF == 0x80 || progIF == 0x8A){
                            *func = function;
                            return (vendor);
                        }
                    }
                }
            }
        }
    } 
    return 0;
}


uint16_t findIDEConrollerDevice(){
    uint16_t bus, ide_bus=0;
    uint8_t device, ide_device=0;
    uint8_t function=0, ide_func=0;

    for(bus = 0; bus < 256; bus++) {
        for(device = 0; device < 32; device++) {
            if(checkifIDE((uint8_t)bus, device, &function)){
                printf("PCI : IDE Controller found(bus=%x, device=%x, func=%x)\n"
                        , bus, device, function);
                return 1;
            }
        }
    }
    return 0;
}