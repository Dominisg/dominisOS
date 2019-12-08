#include <kernel/drvmng.h>
#include <string.h>
#include <stdio.h>
#include <kernel/ata.h>
#include <kernel/fat32.h>

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

void drvmng(char** params){
    uint8_t res, drive, partition;
    // identyfy command
    if (memcmp(params[0], "identify", strlen("identify")) == 0 && strlen(params[0]) == strlen("identify")){
        
        if(params[1][0] == '\0'){ //no third parameter print disks!
            ide_print_summary();
        }else{
            drive = params[1][0]-'0';
            if(drive > 3){
                printf("drvmng: You need to speciify drive id [0-3]! \n");
            }

            if((res = ide_ata_access(ATA_READ, drive, 0, 1,  (uint16_t*) &disk_buffer))!=0){
                return;
            }
                // Copy partiton table
            memcpy(&mbr_partition_table, &(disk_buffer[0x1be]), 64);

            printf("Disk ID[%d] contains:\n", drive);
            for (int i=0; i<4; i++){
                if(mbr_partition_table[i].size>0){
                    if(mbr_partition_table[i].partition_flags == 0x80){
                        printf("[%d] Active partiton size: %dkB\n:w",i, mbr_partition_table[i].size / 2);
                    }
                    else{
                        printf("[%d] Partition size: %dMB\n",i, mbr_partition_table[i].size / 2 / 1024);
                    }
                    printf("    Begin (LBA): %d\n", mbr_partition_table[i].lba_begin);
                    printf("    End (LBA):   %d\n", mbr_partition_table[i].lba_begin + 
                            mbr_partition_table[i].size);
                    printf("    Sectors:     %d\n", mbr_partition_table[i].size);

                    switch(mbr_partition_table[i].file_system_ident){
                        case 0x0c:
                            printf("    File system: FAT32\n");
                        break;
                        case 0x0f:
                            printf("    Extended Partition\n");
                        break;
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
    }else if (memcmp(params[0], "mount", strlen("mount")) == 0 && strlen((params[0])) == strlen("mount")) {
        drive = params[1][0]-'0';
        partition = params[2][0] - '0';
        if(partition > 3 || drive > 3 ){
            printf("drvmng: Specify correct drive and partiton id!\n");
            return;
        }

        if((res = ide_ata_access(ATA_READ, drive, 0, 1,  (uint16_t*) &disk_buffer))!=0){
            return;
        }
            // Copy partiton table
        memcpy(&mbr_partition_table, &(disk_buffer[0x1be]), 64);
        
        if(mbr_partition_table[partition].file_system_ident != 0x0C){
            printf("drvmng: Unsupported file system!\n");
            return;
        }

        fat_mount(drive, mbr_partition_table[partition].lba_begin);
    }
    else{
        printf("drvmng: Unknown command!\n");
    }
}