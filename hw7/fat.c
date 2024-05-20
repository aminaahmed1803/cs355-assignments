#include <stdio.h>
#include "fat.h"

//have a array of file handles
file *all_file_handles[50];
dir *all_dir_handles[50];
int f_idx = 0;
int d_idx = 0;

//have a array of directory handles
int f_init(char *filename){
    if (fat_init==false){
    
        disk = fopen(filename, "r+");
        if (disk == NULL) {
            return FAIL;
        }  

        sb = (super_block*)malloc(sizeof(super_block));
        memset(sb, INITIALIZE, BLOCKSIZE);
        fread(sb,sizeof(super_block),1,disk);
        //printf("sb: %d, %d, %d, \n", sb->data_offset, sb->total_blocks, sb->fat_entries);

        fat_table = (uint16_t*)malloc(sb->fat_size);
        memset(fat_table, INITIALIZE, sb->fat_size);
        fread(fat_table,sizeof(uint16_t),sb->fat_entries,disk);
        //printf("%d\n", fat_table[0]);

        fat_init = true;

        current_directory = (dir_header *)malloc(sizeof(dir_header));
        memset((void *)current_directory, INITIALIZE, BLOCKSIZE);
        fseek(disk,sb->data_offset,SEEK_SET);
        fread(current_directory,sizeof(dir_header),1,disk);
        //printf("%d\n", current_directory->is_dir);

        assert(disk);
        assert(fat_init);
        return SUCCESS;
    }
    return FAIL;
    
}

void update_current_directory(){
    fseek(disk,sb->data_offset + (current_directory->first_FAT * BLOCKSIZE),SEEK_SET);
    fread(current_directory,sizeof(dir_header),1,disk);
}

void sanity_check(file_header *header){

    uint16_t idx = header->first_FAT;
    int offset = sb->data_offset + (idx * BLOCKSIZE);
    file_header *tmp = (file_header *)malloc(sizeof(file_header));
    memset(tmp, INITIALIZE, BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(header,sizeof(file_header),1,disk);

    u_int16_t first_fat = header->first_FAT;
    u_int16_t parent_fat = header->parent_FAT;
    size_t size = header->size;
    u_int16_t cap = -1;

    do {

        memset(tmp, INITIALIZE, BLOCKSIZE);
        offset = sb->data_offset + (idx * BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fread(tmp,sizeof(file_header),1,disk);
        
        strncpy(tmp->name, header->name, 12);
        tmp->uid = header->uid;
        tmp->is_dir = header->is_dir;
        tmp->first_FAT = first_fat;
        tmp->parent_FAT = parent_fat;
        tmp->dir_FAT = header->dir_FAT;
        tmp->size = header->size;
        tmp->total_blocks  = header->total_blocks;

        offset = sb->data_offset + (idx * BLOCKSIZE);
        idx = fat_table[idx];
        size -= 480;
        if (idx != -1){
            tmp->buffer_used = 480;
        } else {
            tmp->buffer_used = size;
        }
        tmp->next_FAT = idx;
        fseek(disk,offset,SEEK_SET);
        fwrite(tmp,sizeof(file_header),1,disk);
        
    } while (idx != cap);

    free(tmp);
    tmp = NULL;
    return;
}

int create_file(char *filename, dir_header *directory){
    //get free FAT
    int idx = 1;
    u_int16_t cap = -1;
    for (; idx<sb->total_data_blocks && fat_table[idx] != 0; idx++){
    }
    if (idx >= sb->total_data_blocks){
        return FAIL;
    }
    if (directory->items >= 15){
        return FAIL;
    }
    //printf("new file name: %s\n", filename);
    strncpy(directory->files[directory->items].name, filename, 12);
    directory->files[directory->items].first_FAT = idx;
    directory->files[directory->items].parent_FAT = idx;
    directory->files[directory->items].size = 0;
    directory->files[directory->items].total_blocks = 1;
    directory->files[directory->items].uid = uid;
    directory->files[directory->items].is_dir = FALSE;
    directory->files[directory->items].init = TRUE;    
    fat_table[idx] = -1;
    directory->items += 1;
    //create a file header 
    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    strncpy(header->name, filename, 12);
    header->uid = uid; 
    header->is_dir = FALSE; 
    header->first_FAT = idx; 
    header->next_FAT = -1;
    header->parent_FAT = idx;
    header->dir_FAT = directory->first_FAT;
    header->total_blocks = 1;
    header->buffer_used = 0;
    header->size = 0;
    memset(header->buffer, INITIALIZE, 480); 
    
    int offset = sb->data_offset + (idx * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(header,sizeof(file_header),1,disk);

    offset = sb->data_offset + (directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(directory,sizeof(dir_header),1,disk);
    
    free(header);
    header=NULL;
    return idx;
}

dir_header * get_directory(char *pathname){ // need to handle . and ..
    if (pathname == NULL){
        return NULL;
    }

    //read in current directory
    dir_header *directory = (dir_header *)malloc(sizeof(dir_header));
    memset((void *)directory, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (current_directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(directory,sizeof(dir_header),1,disk);

    char *token = strtok(pathname, DELIM);
    int idx = -1;
   
    while (token != NULL){
        if (strnlen(token, 12) > 12){
            return NULL;
        } 
        //printf("directory name: %s\n", directory->name);
        //printf("directory items: %d\n", directory->items);
        for (int i=0; i<directory->items; i++){    
            //assert(directory->files[i].init == TRUE);
            //printf("directory file name: %s\n", directory->files[i].name);

            if (strcmp(token, directory->files[i].name) == 0){
                if (uid==SUPERUSER || (uid==USER && directory->files[i].uid==USER) ){
                    idx = i;
                    break;
                }else{
                    printf("access not granted\n");
                    return NULL;
                }
            }
        }
        
        token = strtok(NULL, DELIM);
        //printf("idx: %d\n",idx);
        if(token == NULL && (idx=-1 || directory->files[idx].is_dir==FALSE)){
            return directory;
        }
        if(token != NULL && directory->files[idx].is_dir==FALSE){
            free(directory);
            return NULL;
        }

        offset = sb->data_offset + (directory->files[idx].first_FAT * BLOCKSIZE);
        memset((void *)directory, INITIALIZE, BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fread(directory,sizeof(dir_header),1,disk); 
    }

    return NULL; 
}

/*open the specified file with the specified access (read, write, read/write, append). 
If the file does not exist, handle accordingly. (rule of thumb: create file if 
writing/appending, return error if reading is involved). Returns a file handle if successful.*/
file *f_open(char *pathname, int mode){

    if (!fat_init) {
        return NULL;
    }

    if (pathname == NULL || mode < 1 || mode > 4){
        return NULL;
    }

    int fat = FAIL;
    size_t file_size = 0;
    int len = strlen(pathname);
    char *copy = strcpy( (char *)malloc(len+1), pathname);
    char *cpy2 = strcpy( (char *)malloc(len+1), pathname);
    char *token = strtok(copy, DELIM);
    char *filename = token;
    while (token != NULL){
        filename = token;
        token = strtok(NULL, DELIM);
    } //printf("filename: %s\n", filename);
     
    if (filename == NULL){
        return NULL;
    }

    dir_header *dir = get_directory(cpy2);
    if (dir == NULL){
        return NULL;
    }
     
    if (dir->items >= 15){
        free(copy);
        free(dir);
        return NULL;
    }

   
    for(int i=0; i<dir->items; i++){
        if (strncmp(filename, dir->files[i].name, 12)==0 && dir->files[i].is_dir==FALSE){
            if (dir->files[i].uid != uid && uid != SUPERUSER){
                free(copy);
                free(dir);
                printf("access not granted\n");
                return NULL;

            }
            fat = dir->files[i].first_FAT;
        }
    }
    if(fat==FAIL && (mode==READ_ONLY || mode==READ_WRITE) ){
        free(copy);
        free(dir);
        return NULL;
    }
    //printf("%d\n", fat);
    if (fat==-1){
        //printf("file doesnt exist\n");
        fat = create_file(filename, dir);
    }
    else{
    
        //read in file and get size 
        file_header *header = (file_header *)malloc(sizeof(file_header));
        memset(header, INITIALIZE, BLOCKSIZE);
        int offset = sb->data_offset + (fat * BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fread(header,sizeof(file_header),1,disk);
        file_size = header->size;
        free(header);
        header=NULL;
        //printf("file size: %d\n", header->size);
    }
    
    if (fat==FAIL){
        free(copy);
        free(dir);
        return NULL;
    }

    assert(fat!=FAIL);

    file *handle = (file *)malloc(sizeof(file));
    strncpy(handle->name, filename, 12);
    handle->first_FAT = fat; 
    handle->curr_FAT = fat; 
    handle->mode = mode;
    handle->read_bytes=0; 
    handle->write_bytes=0; 
    handle->size=file_size;
    handle->dir_FAT = dir->first_FAT;

    free(copy);
    copy = NULL;
    free(dir);
    dir = NULL;

    all_file_handles[f_idx] = handle;
    f_idx += 1;

    update_current_directory();

    return handle;
}

/*read the specified number of bytes from a file handle at the current position. 
Returns the number of bytes read, or an error.*/
size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream){
    if (!fat_init){
        return FAIL;
    }

    if (stream == NULL){
        return FAIL;
    }

    if (stream->mode == WRITE_ONLY){
        return FAIL;
    }

    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (stream->curr_FAT * BLOCKSIZE);

    size_t total_bytes = size * nmemb;
    
    if (header->uid == uid || uid == SUPERUSER){
                
        size_t bytes_read = 0;
        uint16_t idx = stream->curr_FAT;

        char *read_buffer = (char *)ptr;

        do {

            fseek(disk,offset,SEEK_SET);
            fread(header,sizeof(file_header),1,disk);

            int already_read = stream->read_bytes;
            int to_read = header->buffer_used - already_read;
            memcpy(read_buffer + bytes_read, header->buffer + already_read, to_read);

            bytes_read += to_read;
            stream->read_bytes += to_read;

            idx = header->next_FAT;
            offset = sb->data_offset + (idx * BLOCKSIZE);

            if (idx != -1){
                stream->curr_FAT = idx;
                stream->read_bytes = 0;
                stream->write_bytes = 0;
            }
        
        } while (idx != -1 && bytes_read < total_bytes);
    
    }
    //free(header);
    header = NULL;
    
    return total_bytes;
}

/*write some bytes to a file handle at the current position. 
Returns the number of bytes written, or an error.*/
size_t f_write(void *ptr, size_t size, size_t nmemb, file *stream){

    if (!fat_init){
        return FAIL;
    }

    if (stream == NULL){
        return FAIL;
    }

    if (stream->mode == READ_ONLY){
        return FAIL;
    }

    uint16_t write_fat = stream->first_FAT;
    uint16_t cap = -1;
    while (fat_table[write_fat] != cap){
        write_fat = fat_table[write_fat];
    }
    //printf("write_fat: %d\n", write_fat);
    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (write_fat * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(header,sizeof(file_header),1,disk); // this is the first file block

    if (header->uid != uid && uid != SUPERUSER){
        printf("access not granted\n");
        free(header);
        header = NULL;
        return FAIL;
    }

    u_int16_t dir_FAT = header->dir_FAT;
    u_int16_t parent_FAT = header->parent_FAT;
    u_int16_t first_FAT = header->first_FAT;
    u_int16_t total_blocks = header->total_blocks;
    u_int32_t file_size = header->size;
    u_int8_t uid = header->uid;

    char *new_data = (char *)ptr;
    size_t total_bytes = size * nmemb;
    int bytes_written = 0;
    //printf("total bytes: %ld\n", total_bytes);

    if (total_bytes <= 0){
        free(header);
        header = NULL;
        return 0;
    }
    for (int i=0; bytes_written < total_bytes; i++){

        int space = 480 - header->buffer_used;
        space = (space > total_bytes - bytes_written) ? total_bytes - bytes_written : space;
        offset = sb->data_offset + (write_fat * BLOCKSIZE);
        memcpy(header->buffer + header->buffer_used, new_data + bytes_written, space);
        header->buffer_used += space;
        header->size += total_bytes;
        fseek(disk,offset,SEEK_SET);
        fwrite(header,sizeof(file_header),1,disk);

        bytes_written += space;

        if (bytes_written < total_bytes){
            int idx = 1;
            for (; idx<sb->total_data_blocks && fat_table[idx] != 0; idx++){
            }
            if (idx >= sb->total_data_blocks){
                return FAIL;
            }
            if (idx == 1){
                return bytes_written;
            }   
            //printf("idx for file %s: %d\n", stream->name, idx);

            header->next_FAT = idx;
            fat_table[write_fat] = idx;
            fat_table[idx] = -1;
            write_fat = idx;
            total_blocks += 1;

            offset = sb->data_offset + (idx * BLOCKSIZE);
            memset(header, INITIALIZE, BLOCKSIZE);
            strncpy(header->name, stream->name, 12); 
            header->uid = uid;
            header->is_dir = FALSE;
            header->dir_FAT = dir_FAT;
            header->parent_FAT = parent_FAT;
            header->first_FAT = first_FAT;
            header->total_blocks = total_blocks;
            header->size = file_size;
            header->buffer_used = 0;
            header->next_FAT = -1;

        }
    }
    header->total_blocks = total_blocks;
    header->size = file_size + total_bytes;

    sanity_check(header);
    stream->curr_FAT = write_fat;
    free(header);
    header = NULL;

    update_current_directory();
    
    return bytes_written;
}

/* close a file handle*/
int f_close(file *stream){

    if (!fat_init){
        return FAIL;
    }

    if (stream == NULL){
        return FAIL;
    }

    free(stream);
    stream=NULL;

    return SUCCESS;
}

/*move to a specified position in a file*/
int f_seek(file *stream, long offset, int position){

    if (!fat_init){
        return FAIL;
    }
    if (stream == NULL){
        return FAIL;
    }

    printf("not implemented\n");

    return FAIL;
}

/*move to the start of the file*/
void f_rewind(file *stream){

    if (!fat_init){
        return;
    }
    if (stream == NULL){
        return;
    }

    if (stream->curr_FAT == stream->first_FAT){
        stream->read_bytes = 0;
        stream->write_bytes = 0;

        return;
    }

}

/* retrieve information about a file*/
int f_stat(file *stream, file_header *stat_buffer){

    if (!fat_init){
        return FAIL;
    }
    if (stream == NULL){
        return FAIL;
    }
    if (stat_buffer == NULL){
        return FAIL;
    }

    memset(stat_buffer, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (stream->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(stat_buffer,sizeof(file_header),1,disk);

    return SUCCESS;
}

//f_remove: delete a file
int f_remove(char *pathname){
    if (!fat_init) {
        return FAIL;
    }

    if (pathname == NULL ){
        return FAIL;
    }

    int fat = FAIL;
    size_t file_size = 0;
    int len = strlen(pathname);
    char *copy = strcpy( (char *)malloc(len+1), pathname);
    char *cpy2 = strcpy( (char *)malloc(len+1), pathname);
    char *token = strtok(copy, DELIM);
    char *filename = token;
    while (token != NULL){
        filename = token;
        token = strtok(NULL, DELIM);
    } //printf("filename: %s\n", filename);
     
    if (filename == NULL){
        return FAIL;
    }

    dir_header *dir = get_directory(cpy2);
    if (dir == NULL){
        free(copy);
        free(dir);
        return FAIL;
    }
     
    if (dir->items <= 0){
        free(copy);
        free(dir);
        return FAIL;
    }
    int idx = -1;
    for(int i=0; i<dir->items; i++){
        if (strncmp(filename, dir->files[i].name, 12)==0 && dir->files[i].is_dir==FALSE){
            if (dir->files[i].uid != uid && uid != SUPERUSER){
                free(copy);
                free(dir);
                printf("access not granted\n");
                return FAIL;

            }
            idx = i;
            fat = dir->files[i].first_FAT;
        }
    }

    if(fat==FAIL || idx==-1){
        printf("no such file\n");
        free(copy);
        free(dir);
        return FAIL;
    }

    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);

    uint16_t cap = -1;
    while (fat_table[fat] != cap){

        int offset = sb->data_offset + (fat * BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fwrite(header,sizeof(file_header),1,disk);

        int tmp = fat;
        fat = fat_table[fat];
        fat_table[tmp] = 0;
    }

    for (int i=idx; i<14; i++){
        strncpy(dir->files[idx].name, dir->files[idx+1].name, 12);
        dir->files[idx].first_FAT = dir->files[idx+1].first_FAT;
        dir->files[idx].parent_FAT = dir->files[idx+1].parent_FAT;
        dir->files[idx].size = dir->files[idx+1].size;
        dir->files[idx].total_blocks = dir->files[idx+1].total_blocks;
        dir->files[idx].uid = dir->files[idx+1].uid;
        dir->files[idx].is_dir = dir->files[idx+1].is_dir;
        dir->files[idx].init = dir->files[idx+1].init;
    }

    idx = 14; 
    strncpy(dir->files[idx].name, "name 00", 12);
    dir->files[idx].init = FALSE;
    dir->files[idx].first_FAT = -1;
    dir->files[idx].parent_FAT = -1;
    dir->files[idx].size = 0;
    dir->files[idx].total_blocks = 0;
    dir->files[idx].uid = -1;
    dir->files[idx].is_dir = FALSE;
    dir->items -= 1;

    int offset = sb->data_offset + (dir->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(dir,sizeof(dir_header),1,disk);

    update_current_directory();

    free(header);
    free(copy);
    free(dir);

    return SUCCESS;
}

/*recall that directories are handled as special cases of files. 
open a “directory file” for reading, and return a directory handle.*/
dir *f_opendir(char *name){

    if (!fat_init){
        return NULL;
    }
    if (name == NULL){
        return NULL;
    }

    for(int i=0; i<current_directory->items; i++){
        if (strncmp(name, current_directory->files[i].name, 12)==0 && current_directory->files[i].is_dir==TRUE){
            if (current_directory->files[i].uid != uid && uid != SUPERUSER){
                printf("access not granted\n");
                return NULL;
            }
            dir *handle = (dir *)malloc(sizeof(dir));
            strncpy(handle->name, name, 12);
            handle->first_FAT = current_directory->files[i].first_FAT;

            dir_header *directory = (dir_header *)malloc(sizeof(dir_header));
            memset((void *)directory, INITIALIZE, BLOCKSIZE);
            int offset = sb->data_offset + (current_directory->files[i].first_FAT * BLOCKSIZE);
            fseek(disk,offset,SEEK_SET);
            fread(directory,sizeof(dir_header),1,disk);

            handle->uid = current_directory->files[i].uid;
            handle->items = directory->items;

            if (handle->items > 0){

                handle->idx = 0; 

                handle->current = (dir_entry *)malloc(sizeof(dir_entry));
                memset((void *)handle->current, INITIALIZE, BLOCKSIZE);
                strcpy(handle->current->name, directory->files[0].name);
                handle->current->first_FAT = directory->files[0].first_FAT;
                handle->current->parent_FAT = directory->files[0].parent_FAT;
                handle->current->size = directory->files[0].size;
                handle->current->total_blocks = directory->files[0].total_blocks;
                handle->current->uid = directory->files[0].uid;
                handle->current->is_dir = directory->files[0].is_dir;
                handle->current->init = directory->files[0].init;
            }

            free(directory);
            directory = NULL;
            return handle;
        }
    }
    return NULL;
}

/*returns a pointer to a “directory entry” structure representing 
the next directory entry in the directory file specified.*/
dir_entry *f_readdir(dir *directory){

    if (!fat_init){
        return NULL;
    }
    if (directory == NULL){
        return NULL;
    }
    if (directory->items == 0){
        return NULL;
    }
    if (directory->idx + 1 >= directory->items){
        return NULL;
    }

    dir_header *tmp = (dir_header *)malloc(sizeof(dir_header));
    memset((void *)tmp, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(tmp,sizeof(dir_header),1,disk);

    directory->idx += 1;

    memset((void *)directory->current, INITIALIZE, BLOCKSIZE);
    //free(directory->current);
    //directory->current = NULL;

    dir_entry *entry = (dir_entry *)malloc(sizeof(dir_entry));
    memset((void *)entry, INITIALIZE, BLOCKSIZE);

    strcpy(entry->name, tmp->files[0].name);
    entry->first_FAT = tmp->files[0].first_FAT;
    entry->parent_FAT = tmp->files[0].parent_FAT;
    directory->current->size = tmp->files[0].size;
    directory->current->total_blocks = tmp->files[0].total_blocks;
    directory->current->uid = tmp->files[0].uid;
    directory->current->is_dir = tmp->files[0].is_dir;
    directory->current->init = tmp->files[0].init;

    free(tmp);
    tmp = NULL;

    return NULL;
}

/* close an open directory file*/
int f_closedir(dir *stream){

    if (!fat_init){
        return FAIL;
    }
    if (stream == NULL){
        return FAIL;
    }

    free(stream->current);
    stream->current = NULL;

    free(stream);
    stream = NULL;

    return SUCCESS;
}

/*make a new directory at the specified location*/
int f_mkdir(char *pathname){

    if (!fat_init){
        return FAIL;
    }
    if (pathname == NULL ){
        return FAIL;
    }

    int fat = FAIL;
    int len = strlen(pathname);
    char *copy = strcpy( (char *)malloc(len+1), pathname);
    char *cpy2 = strcpy( (char *)malloc(len+1), pathname);
    char *tkn = strtok(copy, DELIM);
    char *dirname = tkn;
    while (tkn != NULL){
        dirname = tkn;
        tkn = strtok(NULL, DELIM);
    } //printf("filename: %s\n", filename);

    if (dirname == NULL){
        return FAIL;
    }
    
    dir_header *directory = (dir_header *)malloc(sizeof(dir_header));
    memset((void *)directory, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (current_directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(directory,sizeof(dir_header),1,disk);

    char *token = strtok(cpy2, DELIM);
    
    while (token != NULL){

        if (strnlen(token, 12) > 12){
            return FAIL;
        } 
        int idx = -1;
        //printf("directory name: %s\n", directory->name);
        //printf("directory items: %d\n", directory->items);
        for (int i=0; i<directory->items; i++){    
            //assert(directory->files[i].init == TRUE);
            //printf("directory file name: %s\n", directory->files[i].name);

            if (strcmp(token, directory->files[i].name) == 0){
                if (directory->files[i].is_dir!=TRUE){
                   continue;
                }
                if (uid==SUPERUSER || (uid==USER && directory->files[i].uid==USER) ){
                    idx = i;
                    break;
                }else{
                    printf("access not granted\n");
                    free(directory);
                    return FAIL;
                }
            }
        }
        
        token = strtok(NULL, DELIM);
    
        if(token != NULL && idx==-1){
            printf("invalid path\n");
            free(directory);
            return FAIL;
        }

        if (token == NULL && idx!=-1){
            printf("directory already exists\n");
            
            free(directory);
            directory = NULL;

            free(copy);
            copy = NULL;

            free(cpy2);
            cpy2 = NULL;
            
            return FAIL;
        }
        if(token == NULL && idx==-1 ){
            break;
        }

        offset = sb->data_offset + (directory->files[idx].first_FAT * BLOCKSIZE);
        memset((void *)directory, INITIALIZE, BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fread(directory,sizeof(dir_header),1,disk); 
    }
    
    if (directory->items >= 15){
        printf("directory is full\n");
        free(directory);
        directory = NULL;

        free(copy);
        copy = NULL;

        free(cpy2);
        cpy2 = NULL;

        return FAIL;
    }

    //get free FAT
    int idx = 1;
    u_int16_t cap = -1;
    for (; idx<sb->total_data_blocks && fat_table[idx] != 0; idx++){
    }
    if (idx >= sb->total_data_blocks){
        return FAIL;
    }
    //printf("idx: %d\n",idx);
    //printf("new file name: %s\n", filename);
    strncpy(directory->files[directory->items].name, dirname, 12);
    directory->files[directory->items].first_FAT = idx;
    directory->files[directory->items].parent_FAT = idx;
    directory->files[directory->items].size = 0;
    directory->files[directory->items].total_blocks = 1;
    directory->files[directory->items].uid = uid;
    directory->files[directory->items].is_dir = TRUE;
    directory->files[directory->items].init = TRUE;    
    fat_table[idx] = -1;
    directory->items += 1;
    
    //create a file header 
    dir_header *header = (dir_header *)malloc(sizeof(dir_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    strncpy(header->name, dirname, 12);
    header->uid = uid; 
    header->is_dir = FALSE; 
    header->items = 0;
    header->first_FAT = idx; 
    header->parent_FAT = directory->first_FAT;
    header->idx = 0;
    
    for (int i=0; i<15; i++){
        header->files[i].init = FALSE;
        strncpy(header->files[i].name, "name 00", 12);
        header->files[i].first_FAT = -1;
        header->files[i].parent_FAT = -1;
        header->files[i].size = 0;
        header->files[i].total_blocks = 0;
        header->files[i].uid = -1;
        header->files[i].is_dir = FALSE;
    }

    offset = sb->data_offset + (directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(directory,sizeof(dir_header),1,disk);

    offset = sb->data_offset + ((idx) * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(header,sizeof(dir_header),1,disk);

    offset = sb->data_offset + ((idx) * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(header,sizeof(dir_header),1,disk);
    
    
    free(header);
    header=NULL;

    free(directory);
    directory = NULL;

    free(copy);
    copy = NULL;

    free(cpy2);
    cpy2 = NULL;

    update_current_directory();

    return SUCCESS ; 
}

int rmdir_helper(dir_header *directory){
    int offset = sb->data_offset + (directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(directory,sizeof(dir_header),1,disk);

    for (int i=0; i<directory->items; i++){

        fat_table[directory->files[i].first_FAT] = 0;

        if (directory->files[i].is_dir==TRUE){
            //read in directory
            dir_header *sub_dir = (dir_header *)malloc(sizeof(dir_header));
            memset((void *)sub_dir, INITIALIZE, BLOCKSIZE);
            offset = sb->data_offset + (directory->files[i].first_FAT * BLOCKSIZE);
            fseek(disk,offset,SEEK_SET);
            fread(sub_dir,sizeof(dir_header),1,disk);
            //if directory is not empty
            rmdir_helper(sub_dir);
            //write directory back to disk
            memset((void *)sub_dir, INITIALIZE, BLOCKSIZE);
            fseek(disk,offset,SEEK_SET);
            fwrite(sub_dir,sizeof(dir_header),1,disk);
            free(sub_dir);
            sub_dir = NULL;
        }else {
            file_header *header = (file_header *)malloc(sizeof(file_header));
            memset(header, INITIALIZE, BLOCKSIZE);
            offset = sb->data_offset + (directory->files[i].first_FAT * BLOCKSIZE);
            fseek(disk,offset,SEEK_SET);
            fwrite(header,sizeof(file_header),1,disk);
            fat_table[header->first_FAT] = 0;
            free(header);
            header = NULL;
        }
        
    }
    
    return SUCCESS;
}
/*delete a specified directory. Be sure to remove entire contents and the contents of 
all subdirectorys from the filesystem. Do NOT simply remove pointer to directory.*/
int f_rmdir(char *pathname){

    if (!fat_init){
        return FAIL;
    }
    if (pathname == NULL){
        return FAIL;
    }

    int fat = FAIL;
    int parent_dir_FAT = FAIL;
    int len = strlen(pathname);
    char *copy = strcpy( (char *)malloc(len+1), pathname);
    char *cpy2 = strcpy( (char *)malloc(len+1), pathname);
    char *tkn = strtok(copy, DELIM);
    char *dirname = tkn;
    while (tkn != NULL){
        dirname = tkn;
        tkn = strtok(NULL, DELIM);
    } //printf("filename: %s\n", filename);

    if (dirname == NULL){
        return FAIL;
    }
    
    dir_header *directory = (dir_header *)malloc(sizeof(dir_header));
    memset((void *)directory, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (current_directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(directory,sizeof(dir_header),1,disk);

    char *token = strtok(cpy2, DELIM);
    int idx = -1;
    while (token != NULL){

        if (strnlen(token, 12) > 12){
            return FAIL;
        } 
        idx = -1;
        //printf("directory name: %s\n", directory->name);
        //printf("directory items: %d\n", directory->items);
        for (int i=0; i<directory->items; i++){    
            //assert(directory->files[i].init == TRUE);
            //printf("directory file name: %s\n", directory->files[i].name);

            if (strcmp(token, directory->files[i].name) == 0){
                if (directory->files[i].is_dir!=TRUE){
                   continue;
                }
                if (uid==SUPERUSER || (uid==USER && directory->files[i].uid==USER) ){
                    idx = i;
                    break;
                }else{
                    printf("access not granted\n");
                    free(directory);
                    directory = NULL;

                    free(copy);
                    copy = NULL;

                    free(cpy2);
                    cpy2 = NULL;

                    return FAIL;
                }
            }
        }
        
        token = strtok(NULL, DELIM);
        //printf("idx: %d\n",idx);
        if(token != NULL && idx==-1){
            printf("invalid path\n");
            
            free(directory);
            directory = NULL;

            free(copy);
            copy = NULL;

            free(cpy2);
            cpy2 = NULL;
            return FAIL;
        }

        if (token == NULL && idx!=-1){
            rmdir_helper(directory);
            break;
        }
        if(token == NULL && idx==-1 ){
            printf("directory doesnt exist\n");

            free(directory);
            directory = NULL;

            free(copy);
            copy = NULL;

            free(cpy2);
            cpy2 = NULL;

            return FAIL;
        }

        offset = sb->data_offset + (directory->files[idx].first_FAT * BLOCKSIZE);
        memset((void *)directory, INITIALIZE, BLOCKSIZE);
        fseek(disk,offset,SEEK_SET);
        fread(directory,sizeof(dir_header),1,disk); 
    }
    
    for (int i=idx; i<14; i++){
        strncpy(directory->files[idx].name, directory->files[idx+1].name, 12);
        directory->files[idx].first_FAT = directory->files[idx+1].first_FAT;
        directory->files[idx].parent_FAT = directory->files[idx+1].parent_FAT;
        directory->files[idx].size = directory->files[idx+1].size;
        directory->files[idx].total_blocks = directory->files[idx+1].total_blocks;
        directory->files[idx].uid = directory->files[idx+1].uid;
        directory->files[idx].is_dir = directory->files[idx+1].is_dir;
        directory->files[idx].init = directory->files[idx+1].init;
    }

    idx = 14; 
    strncpy(directory->files[idx].name, "name 00", 12);
    directory->files[idx].init = FALSE;
    directory->files[idx].first_FAT = -1;
    directory->files[idx].parent_FAT = -1;
    directory->files[idx].size = 0;
    directory->files[idx].total_blocks = 0;
    directory->files[idx].uid = -1;
    directory->files[idx].is_dir = FALSE;
    directory->items -= 1;

    offset = sb->data_offset + (directory->first_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fwrite(directory,sizeof(dir_header),1,disk);

    free(directory);
    directory = NULL;

    free(copy);
    copy = NULL;

    free(cpy2);
    cpy2 = NULL;

    update_current_directory();
            
    return SUCCESS;

}

void f_terminate(){
    
    if (!fat_init){
        return;
    }
    fseek(disk, BLOCKSIZE, SEEK_SET);
    fwrite(fat_table,sizeof(uint16_t),sb->fat_entries,disk);

    free(sb);
    free(fat_table);
    free(current_directory);

    sb = NULL;
    fat_table = NULL;
    current_directory = NULL;

    fclose(disk);
}

/*ls lists all the files in the current or specified directory. Support flags -F and -l.*/
int list_dir(char **command){
    //go through current directoruy and list according to flag present in command
    // or of the sub dir and parents

    if (current_directory == NULL){
        printf("current directory not set\n");
        return FAIL;
    }

    if (current_directory->items <= 0){
        printf("\n");
        return SUCCESS;
    }
    
    for (int i=0; i<current_directory->items; i++){
        
        printf("%s\n", current_directory->files[i].name);
    }
    return SUCCESS;
}