#include <kernel/drvmng.h>
#include <string.h>
#include <stdio.h>
#include <kernel/ata.h>

uint8_t disk_buffer[512];

struct mbr_partiton_table_entry{
    uint8_t  partition_flags;
    uint8_t  chs_head_begin;
    uint16_t chs_cylinder_sector_begin;
    uint8_t  file_system_ident;
    uint8_t  chs_head_end;
    uint16_t chs_cylinder_sector_end;
    uint32_t lba_begin;
    uint32_t size;
} mbr_partition_table[4];

void drvmng(uint8_t** params){
    int8_t res;
    // identyfy command
    if (memcmp(params[0], "identify", strlen("identify")) == 0){
        
        if(params[1][0] == '\0'){ //no third parameter print disks!
            ide_print_summary();
        }else{
            if(params[1][0]-'0'<0 || params[1][0] - '0'> 3){
                printf("drvmng: You need to speciify drive id s[0-3]! \n");
            }
            if((res = ide_ata_access(ATA_READ, params[1][0]-'0', 0, 1,  (uint16_t*) &disk_buffer))==0){
                // Copy partiton table
                memcpy(&mbr_partition_table, &(disk_buffer[0x1be]), 64);
                printf("Disk ID[%d] contains:\n", params[1][0]-'0');
                for (int i=0; i<4; i++){
                    if(mbr_partition_table[i].size>0){
                        if(mbr_partition_table[i].partition_flags == 0x80){
                            printf("[%d] Active partiton size: %dkB\n:w",i, mbr_partition_table[i].size / 2);
                        }
                        else{
                            printf("[%d] Partition size: %dkB\n",i, mbr_partition_table[i].size / 2);
                        }
                        printf("    Begin (LBA): %x\n", mbr_partition_table[i].lba_begin);
                        switch(mbr_partition_table[i].file_system_ident){
                            case 0x0c:
                                printf("    File system: FAT32\n");
                            break;
                            case 0x0f:
                                printf("    Extended Partition\n");
                            case 0x83:
                                printf("    File system: ext2\n");
                            break;
                            default:
                                printf("    Unknown file system\n");
                            break;
                        }
                    }
                }
            }
        }
    }else{
        printf("drvmng: Unknown command!\n");
    }
}