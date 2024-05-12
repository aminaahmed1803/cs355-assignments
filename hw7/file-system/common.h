#ifndef COMMON_H
#define COMMON_H

#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../../../../usr/include/x86_64-linux-gnu/sys/types.h"

//errors
#define E_FILE_NOT_FOUND 2
#define E_OUT_OF_BOUNDS 3
#define E_NOT_DIR 4
#define E_NO_SPACE 5
#define E_PERMISSION_DENIED 6
#define E_FILE_ALREADY_OPEN 7
#define E_NOT_FILE 8
#define E_BAD_NAME 9

//modes
#define READ_ONLY 1
#define WRITE_ONLY 2
#define READ_WRITE 3
#define APPEND 4

#define NUM_OPEN_FILES 50
#define BLOCK_SIZE 512
#define NUM_FAT_ENTRIES 1024

#define NAME_BYTES 9
#define TOTAL_BYTES 1048576
#define TOTAL_BLOCKS 2048 // 1048576 / 512
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
#define ROOTDIR_BYTES 512
#define MYDIR_BYTES 512
#define SUPERBLOCK_PADDING 496
#define FILE_AFTER_HEADER_BYTES 480
#define TABLE_OFFSET 1
#define TABLE_BLOCKS 16
#define FIXED_FREEBLOCK 2
#define UNUSED_BLOCK -2
#define TRUE 1
#define FALSE 0

#define FREE_DATABLOCK_EXTRA_BYTES 508
#define PROT_BYTES 11
#define NONE_FREE -1
#define FILE_HEADER_BYTES 32
#define DIR_ENTRY_BYTES 32

u_int16_t *fat_table;

typedef struct boot_block{ 
    u_int32_t disk_size; 
    u_int32_t total_blocks;
    u_int32_t fat_size;
    u_int32_t fat_offset;
    u_int32_t data_offset;
    char dir_tree[41][12];
    
}boot_block;

typedef struct dir_entry{ // needs to be 32 or 16
    char *name[12];
    u_int16_t first_FAT; 
    u_int32_t size;
    u_int32_t total_blocks;
    u_int8_t uid; 
    u_int8_t is_dir;  
    char junk[8]; 
    
}dir_entry;

//file handle
typedef struct file_handle{ 
    char name[12]; 
    u_int8_t is_dir; 
    u_int16_t first_FAT; 
    u_int16_t curr_FAT; 
    size_t size;
    u_int16_t first_FAT_idx; 
}FILE;

typedef struct dir_handle { 
    char name[12];
    u_int8_t uid; 
    u_int32_t items; 
    u_int16_t first_FAT; 
    u_int32_t idx;
    dir_entry * current; 
}DIR;

typedef struct file_header { 
    char name[12]; 
    u_int8_t uid; 
    u_int8_t is_dir; 
    u_int16_t first_FAT; 
    u_int16_t next_FAT; 
    u_int32_t size; 
    char junk[10]; 
    char *buffer[480]; //480
}file_header;

typedef struct dir_header { 
    char name[12];
    u_int8_t uid; 
    u_int8_t is_dir;
    u_int32_t items; 
    u_int16_t first_FAT; 
    char junk[16]; 
    dir_entry files[15]; 
}dir_header;

void f_init();
file_handle *f_open(const char *pathname, const int mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, file_handle *stream);
size_t f_write(const void *ptr, size_t size, size_t nmemb, file_handle *stream);
int f_close(file_handle *stream);
int f_seek(file_handle *stream, long offset, int position);
void f_rewind(file_handle *stream);
int f_stat(file_handle *stream, file_header *stat_buffer);
int f_remove(const char *pathname);
dir_handle *f_opendir(const char *name);
dir_entry *f_readdir(dir_handle *directory);
int f_closedir(dir_handle *stream);
int f_mkdir(const char *pathname, char *mode);
int f_rmdir(const char *pathname);
void f_terminate();

extern int f_error;
extern FILE *disk;
extern int uid;


#endif 