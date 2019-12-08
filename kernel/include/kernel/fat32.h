#ifndef _FAT_32_H
#define _FAT_32_H 1
#include <stddef.h>
#include <stdint.h>
uint8_t fat_list_rootdir(uint8_t drive, uint32_t partition_begin_lba);
void fat_mount(uint8_t disk, uint32_t lba_begin);
#endif 