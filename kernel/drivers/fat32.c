#include <kernel/fat32.h>
#include <kernel/vfs.h>
#include <kernel/ata.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFFER_SECTORS 2 // Using because don't have heap implemented yet
#define FAT_ATTR_FOLDER 0x10


uint8_t disk_buffer[512*BUFFER_SECTORS];
static uint8_t drive;
static uint32_t partition_begin_lba;

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
    uint32_t volume_serial_number;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
} __attribute__((packed))fat_partition_info;


struct fat_file_descriptor{
    int8_t filename[11];
    uint8_t attrib;
    uint8_t reserved;
    uint8_t creation_time_prec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) directory[16];

void fat_get_partition_info(){
        if(fat_partition_info.bytes_per_sector == 0){
        if((ide_ata_access(ATA_READ, drive, partition_begin_lba, 1,  (uint16_t*) &disk_buffer))!=0){
            return;
        }

        memcpy(&fat_partition_info, &(disk_buffer[0x0b]), sizeof(fat_partition_info));
    }
}


void fat_read_dir(struct vfs_file* vfs_dir, struct vfs_file* vfs_dir_file){
    uint8_t res;
    uint8_t virt_iter = 0;
    uint32_t dir_lba;

    fat_get_partition_info();

    uint32_t fat_begin_lba = partition_begin_lba + fat_partition_info.reserved_sectors;
    uint32_t cluster_begin_lba = fat_begin_lba + (fat_partition_info.fat_copies * fat_partition_info.logical_sectors_per_fat);

    if(vfs_dir_file == NULL){ // read root
        dir_lba = cluster_begin_lba + (fat_partition_info.root_dir_cluster - 2) 
                * fat_partition_info.sectors_per_cluster;
    }else{
        uint8_t ref = vfs_dir_file->fsystem_ref; 
        if(!(vfs_dir_file->attr &= VFS_ATTR_FOLDER))
        {
            printf("File is not a directory!\n");
            return;
        }
        dir_lba = cluster_begin_lba + ((directory[ref].first_cluster_low | (uint32_t)(directory[ref].first_cluster_high << 16)) - 2) 
                * fat_partition_info.sectors_per_cluster;
    }

    if((res = ide_ata_access(ATA_READ, drive, dir_lba, 1,  (uint16_t*) &directory))!=0){
        return;
    }

    int8_t filename_buffer[64];
    filename_buffer[0] = 0;

    memset(vfs_dir, 0, DIR_MAX_SIZE * sizeof(struct vfs_file));

    for (size_t i = 0; i < 16; i++){
        if (directory[i].filename[0] == 0)
            break;
        if ((uint8_t)directory[i].filename[0] == 0xe5)
            continue;
        if(directory[i].attrib == 0xf){ //Long filename record
            uint8_t pos  = directory[i].filename[0] & 0xF;
            uint32_t cnt = 13 *(pos - 1);
            for(size_t j = 1; j < 11; j += 2){
                filename_buffer[cnt++] = *(directory[i].filename + j);  
            }
            for(size_t j = 14; j < 25; j += 2){
                filename_buffer[cnt++] = *(directory[i].filename + j);
            }
            for(size_t j = 28; j < 32; j += 2){
                filename_buffer[cnt++] = *(directory[i].filename + j);
            }
        }else{
            if(filename_buffer[0] == 0){
                memcpy(vfs_dir[virt_iter].filename, directory[i].filename, sizeof(vfs_dir[virt_iter].filename));
                vfs_dir[virt_iter].filename[11] = 0;
                for (size_t j = 0; j<11; j++){
                    if(vfs_dir[virt_iter].filename[j] == ' '){
                        vfs_dir[virt_iter].filename[j] = 0;
                        break;
                    }
                }
            }else{
                memcpy(vfs_dir[virt_iter].filename, filename_buffer, sizeof(vfs_dir[virt_iter].filename));
            }

            vfs_dir[virt_iter].size = directory[i].file_size;
            vfs_dir[virt_iter].fsystem_ref = i;
            if(directory[i].attrib & FAT_ATTR_FOLDER)
                vfs_dir[virt_iter].attr |= VFS_ATTR_FOLDER;
            virt_iter++;
            if(virt_iter == DIR_MAX_SIZE)
                return;
            filename_buffer[0] = 0;
        }
    }    

}

uint8_t* fat_read_file(struct vfs_file* file){
    uint32_t file_lba;
    uint8_t ref;
    fat_get_partition_info();

    uint32_t fat_begin_lba = partition_begin_lba + fat_partition_info.reserved_sectors;
    uint32_t cluster_begin_lba = fat_begin_lba + (fat_partition_info.fat_copies * fat_partition_info.logical_sectors_per_fat);

        ref = file->fsystem_ref; 
        if((file->attr &= VFS_ATTR_FOLDER))
        {
            printf("File is a directory!\n");
            return;
        }
    file_lba = cluster_begin_lba + ((directory[ref].first_cluster_low | (uint32_t)(directory[ref].first_cluster_high << 16)) - 2) 
                * fat_partition_info.sectors_per_cluster;
    
    if((ide_ata_access(ATA_READ, drive, file_lba, 1,  (uint16_t*) &disk_buffer))!=0){
        return NULL;
    }
    if(file->size < 512){
        disk_buffer[file->size] = 0;
    }else{
        disk_buffer[512] = 0;
    }

    return disk_buffer;
}

void fat_mount(uint8_t disk, uint32_t lba_begin){
    drive = disk;
    partition_begin_lba = lba_begin;
    vfs_mount(fat_read_dir, fat_read_file);
    memset(&fat_partition_info, 0, sizeof(fat_partition_info));
}


