#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "fat.h"

//HOW TO RUN
//gcc -o format format.c disk.c
//./format DISK
// *formats a 1 MB disk*
//./format DISK -s 2
// *formats a 2 MB disk*

#define DEFAULT_DISK_SIZE_MB 1
#define FAT_ENTRY_SIZE 2
#define RESERVED_SECTORS 2

void format(const char *filename, int mb) {
    
    if (filename == NULL) {
        return;
    }
    
    if (mb < 1 || mb > 50) {
        return;
    }

    long disk_bytes = mb * KBYTE * KBYTE;
    long fat_entries = disk_bytes / BLOCKSIZE;
    long fat_size = fat_entries * sizeof(uint16_t);
    long data_blocks = fat_entries - RESERVED_SECTORS - (fat_size/BLOCKSIZE);


    uint16_t *fat_table = (uint16_t *)malloc( fat_size); //fat table
    if (fat_table == NULL) {
        return;
    }
    memset((void *)fat_table, INITIALIZE, fat_size);
    fat_table[0] = -1;


    file_header *block = (file_header *)malloc(BLOCKSIZE); //data block
    if (block == NULL) {
        return;
    }
    memset((void *)block, INITIALIZE, BLOCKSIZE);
    
    super_block *sb = (super_block*)malloc(sizeof(super_block)); //boot block
    memset((void *)sb, INITIALIZE, BLOCKSIZE);
    sb->disk_mb = mb;
    sb->total_blocks = fat_entries;
    sb->fat_size = fat_size;
    sb->total_data_blocks = data_blocks;
    sb->fat_offset = BLOCKSIZE;
    sb->data_offset = fat_size + BLOCKSIZE;    
    sb->fat_entries = fat_entries;

    dir_header *tmp = (dir_header*)malloc(sizeof(dir_header)); //root directory
    memset((void *)tmp, INITIALIZE, BLOCKSIZE);
    strncpy(tmp->name, "root_dir", 12);
    tmp->uid = USER; 
    tmp->is_dir = TRUE; 
    tmp->first_FAT = 0;
    tmp->items = 0; 
    tmp->parent_FAT = 0;
    
    for (int i=0; i<15; i++){
        tmp->files[i].init = FALSE;
        strncpy(tmp->files[i].name, "name 00", 12);
        tmp->files[i].first_FAT = -1;
        tmp->files[i].parent_FAT = -1;
        tmp->files[i].size = 0;
        tmp->files[i].total_blocks = 0;
        tmp->files[i].uid = -1;
        tmp->files[i].is_dir = FALSE;
    }
    
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        return;
    }    
    
    fwrite(sb, sizeof(super_block), 1, file);
    
    fwrite(fat_table,sizeof(uint16_t), fat_entries, file);

    fwrite(tmp, sizeof(dir_header), 1, file);

    for(int i = 0; i < data_blocks; i++) {
        fwrite(block, sizeof(file_header), 1, file);
    }

    printf("fat_Table: %ld\n",fat_size);
    printf("block: %ld\n",  sizeof(*block));
    printf("sb: %ld\n", sizeof(*sb));
    printf("root_dir: %ld\n", sizeof(*tmp));

    printf("dir entry: %ld\n", sizeof(dir_entry));
    printf("file handle: %ld\n", sizeof(file));
    printf("dir handle: %ld\n", sizeof(dir));
    printf("file header: %ld\n", sizeof(file_header));
    printf("dir header: %ld\n", sizeof(dir_header));

    free(sb);
    free(fat_table);
    free(block);
    free(tmp);

    tmp = NULL; 
    sb = NULL;
    fat_table = NULL;
    block = NULL;

    fclose(file);    
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Usage: %s <filename> [-s size_mb]\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    format(filename, atoi(argv[3]));
    return 0;
}
