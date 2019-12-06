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

struct fat_bpb{
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_copies;
    uint16_t root_entries;
    uint16_t num_sectors;
    uint8_t  media_type; //descriptor description
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t large_sectors;
    uint32_t logical_sectors_per_fat;
    uint16_t mirroring_flags;
    uint16_t version;
    uint32_t root_dir_cluster;
    uint16_t fs_information_sector;
    uint16_t backup_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  flags;
    uint8_t  extended_boot_signature;
    uint16_t volume_serial_number;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
}fat_partition_info;


void drvmng(uint8_t** params){
    uint8_t res, drive, partition;
    // identyfy command
    if (memcmp(params[0], "identify", strlen("identify")) == 0){
        
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


            if(params[2][0] == '0'){  //NOT exists partition id identifier
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
            }else{
                partition = params[2][0] - '0';
                if(partition> 3){
                    printf("drvmng: Partition id should be in range [0-3]! \n");
                }else
                {
                    if((res = ide_ata_access(ATA_READ, params[1][0]-'0', mbr_partition_table[partition].lba_begin, 1,  (uint16_t*) &disk_buffer))!=0){
                        return res;
                    }

                    memcpy(&fat_partition_info, &(disk_buffer[0x0b]), 64);

                    printf("Drive [%d] - Partition [%d]",drive, partition);
                    printf("     %s", (char*)fat_partition_info.file_system_type);


                }
                

            }
        }
    }else{
        printf("drvmng: Unknown command!\n");
    }
}