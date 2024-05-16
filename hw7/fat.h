#ifndef FAT_H
#define FAT_H

#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <assert.h>

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
#define BLOCKSIZE 512
#define KBYTE 1024

#define NAME_BYTES 12
#define SUPERBLOCK_BYTES 512
#define FATTABLE_BYTES 8192 // 2048 * 4
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

#define USER 1
#define SUPERUSER 0

#define FAIL -1
#define SUCCESS 0

#define INITIALIZE 0

#define DELIM "/"

typedef struct super_block{ 
    u_int32_t disk_mb; 
    u_int32_t total_blocks;
    u_int32_t fat_size;
    u_int32_t fat_entries;
    u_int32_t fat_offset;
    u_int32_t data_offset;
    u_int32_t total_data_blocks;
    char junk[484]; 
    
}super_block;

typedef struct dir_entry{ // needs to be 32 or 16
    char name[12];
    u_int16_t first_FAT; 
    u_int16_t parent_FAT;
    u_int32_t size;
    u_int32_t total_blocks;
    u_int8_t uid; 
    u_int8_t is_dir;  
    u_int32_t init;      
}dir_entry;

//file handle
typedef struct file_handle{ 
    char name[12]; 
    size_t size;
    u_int8_t mode;
    u_int16_t first_FAT; 
    u_int16_t curr_FAT; 
    u_int16_t dir_FAT; 
    u_int16_t read_bytes; 
    u_int16_t write_bytes; 

}file;

typedef struct dir_handle { 
    char name[12];
    u_int8_t uid; 
    u_int32_t items; 
    u_int16_t first_FAT; 
    u_int32_t idx;
    dir_entry * current; 
}dir;

typedef struct file_header { 
    char name[12]; //12
    u_int8_t uid; //1
    u_int8_t is_dir; //1
    u_int16_t first_FAT; //2 
    u_int16_t next_FAT;  //2
    u_int16_t parent_FAT; //2
    u_int16_t buffer_used; //2
    u_int16_t dir_FAT; //2
    u_int16_t total_blocks; //2
    u_int32_t size; //4
    u_int16_t unused; //2
    char buffer[480]; //480
}file_header;

typedef struct dir_header { 
    char name[12];
    u_int8_t uid; 
    u_int8_t is_dir;
    u_int32_t items; 
    u_int16_t first_FAT; 
    u_int16_t parent_FAT; 
    dir_entry files[15]; 
    char junk[6];
}dir_header;

u_int16_t *fat_table;
super_block *sb;
int f_error;
FILE *disk;
int uid;

int f_init(int uid, char *filename);
file *f_open(char *pathname, int mode);
size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream);
size_t f_write(void *ptr, size_t size, size_t nmemb, file *stream);
int f_close(file *stream);
int f_seek(file *stream, long offset, int position);
void f_rewind(file *stream);
int f_stat(file *stream, file_header *stat_buffer);
int f_remove(const char *pathname);
dir *f_opendir(const char *name);
dir_entry *f_readdir(dir *directory);
int f_closedir(dir *stream);
int f_mkdir(const char *pathname, char *mode);
int f_rmdir(const char *pathname);
void f_terminate();

#endif 