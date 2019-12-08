#ifndef _VFS_H
#define _VFS_H 1
#include <stddef.h>
#include <stdint.h>

#define VFS_ATTR_FOLDER 0x01
#define DIR_MAX_SIZE 64

struct vfs_file{
    char filename[DIR_MAX_SIZE];
    uint8_t attr;
    uint8_t fsystem_ref;
    uint32_t size;
};

typedef void (*vfs_read_dir_f)(struct vfs_file*, struct vfs_file*);
typedef int8_t* (*vfs_read_file_f)(struct vfs_file);

void ls(char** params);
void vfs_mount(void* read_dir_fnc, void* read_file_fnc);
void cd(char** params);

#endif 