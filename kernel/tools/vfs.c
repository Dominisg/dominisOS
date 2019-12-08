#include <kernel/vfs.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <string.h>
#include <stdio.h>

static struct vfs_file current_dir[DIR_MAX_SIZE];
static vfs_read_dir_f read_dir_ptr=NULL;
static vfs_read_file_f read_file_ptr=NULL;

void vfs_read_dir(struct vfs_file* dir, struct vfs_file* file){
    if(read_dir_ptr != NULL){
        read_dir_ptr(dir, file);
    }
}

void ls(char** params){
    uint8_t i=0, long_mode=0;

    if(params[0][0] == '-'){
        while(params[0][i]){
            
            switch(params[0][i]){
                case 'l':
                    long_mode = 1;
                break;
                default:
                break;
            }
            i++;
        }
    }

    if(current_dir[0].filename[0]==0){
        vfs_read_dir(current_dir, NULL);
    }

    for (size_t i = 0; i < DIR_MAX_SIZE; i++){
        if(current_dir[i].filename[0] == 0){
            if(i!=0 && !long_mode){
                printf("\n");
            }
            break;
        }
        if(current_dir[i].attr & VFS_ATTR_FOLDER){
            terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
        }
        printf("%s ", current_dir[i].filename);

        if(current_dir[i].attr & VFS_ATTR_FOLDER){
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        }

        if(long_mode){
            printf("%d B\n", current_dir[i].size);
        }
    }
}

void cd(char** params){

    if(current_dir[0].filename[0]==0){
        vfs_read_dir(current_dir, NULL);
    }

    for (size_t i = 0; i < DIR_MAX_SIZE; i++){
        if(current_dir[i].filename[0] == 0){
            break;
        }
        if(memcmp(params[0], current_dir[i].filename, strlen(current_dir[i].filename)) == 0
            && strlen(params[0]) == strlen(current_dir[i].filename)){
            vfs_read_dir(current_dir, &current_dir[i]);
            break;
        }
    }
}

void vfs_mount(void* read_dir_fnc, void* read_file_fnc){
    read_dir_ptr = (vfs_read_dir_f)read_dir_fnc;
    read_file_ptr = (vfs_read_file_f)read_file_fnc;
}