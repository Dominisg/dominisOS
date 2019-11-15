#include <kernel/acpi.h>
#include <string.h>
#include <stdio.h>

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

struct RSDT {
  struct ACPISDTHeader h;
  uint32_t PointerToOtherSDT[];
};

struct XSDT {
  struct ACPISDTHeader h;
  uint64_t PointerToOtherSDT[];
};


struct FADT* fadtptr = NULL;

static uint32_t findRSDPDescriptor(){
   struct RSDPDescriptor20 *rsdp = NULL;
   uint16_t *edba_ptr_location = (uint16_t*) 0x40E;
   char *edba = (char*)(uint32_t) *edba_ptr_location;
   char *ptr = edba;


    for (uint32_t i = 0; i < 1024; i++){
        if(memcmp(ptr, "RSD PTR ", 8) == 0 ){
            rsdp = (struct RSDPDescriptor20 *) ptr;
            //printf("RSDPtr : %x\n", (uint32_t) rsdp);
            return (uint32_t) rsdp;
        }
        ptr++;
    }
    ptr = (char*)0xE0000;
    while(ptr != (char*)0xFFFFF){
        if(memcmp(ptr, "RSD PTR ", 8) == 0){
            rsdp = (struct RSDPDescriptor20 *) ptr;
            //printf("RSDPtr : %x\n", (uint32_t) rsdp);
            return (uint32_t) rsdp;
        }
        ptr++;
    }

    return 0;
}

static int8_t acpiDoChecksum(struct ACPISDTHeader *tableHeader)
{
    unsigned char sum = 0;
 
    for (uint32_t i = 0; i < tableHeader->Length; i++)
    {
        sum += ((char *) tableHeader)[i];
    }
 
    return sum == 0;
}

static uint32_t findFADT(void *RootSDT, uint8_t revision)
{

    if (revision != 0){
        struct XSDT *xsdt = (struct XSDT *) RootSDT;
        uint32_t entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;
    
        for (uint32_t i = 0; i < entries; i++)
        {
            struct ACPISDTHeader *h = (struct ACPISDTHeader *)(uint32_t) xsdt->PointerToOtherSDT[i];
            if (memcmp(h->Signature, "FACP", 4) == 0)
                    return  acpiDoChecksum(h) ? (uint32_t) h : 0; 
        }
    
        
    } else{
        struct RSDT *rsdt = (struct RSDT *) RootSDT;
        uint32_t entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
        for (uint32_t i = 0; i < entries; i++)
        {
            struct ACPISDTHeader *h = (struct ACPISDTHeader *) rsdt->PointerToOtherSDT[i];
            if (memcmp(h->Signature, "FACP", 4) == 0){
                return  acpiDoChecksum(h) ? (uint32_t) h : 0; 
            }
        }
    }

    // No FACP found
    return 0;
}

uint32_t getFADT(){
    if(fadtptr != 0){
        return (uint32_t)fadtptr;
    }

    struct RSDPDescriptor20 *rsdp = NULL;
    void *RootSDT;

    if((rsdp = (void *)findRSDPDescriptor())==0){
        return 0;
    }

    if(rsdp->firstPart.Revision == 0){
        if(!acpiDoChecksum((void*)rsdp->firstPart.RsdtAddress)){
            return 0;
        }
        RootSDT = (void*)rsdp->firstPart.RsdtAddress;
        //printf("This PC has RSDT, address:%x\n", RootSDT);
    }else{
        if(!acpiDoChecksum((void*)(uint32_t)rsdp->XsdtAddress)){
            return 0;
        }
        RootSDT = (void*)(uint32_t)rsdp->XsdtAddress;
        //printf("This PC has XSDT, address:%x\n", RootSDT);
    }

    return findFADT(RootSDT, rsdp->firstPart.Revision);
}

