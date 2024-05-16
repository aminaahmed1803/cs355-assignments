#include <stdio.h>
#include "fat.h"

bool init = false; 
dir_header *current_directory; 

int f_init(int uid, char *filename){
    if (!init){
    
        disk = fopen(filename, "r+");
        if (disk == NULL) {
            return FAIL;
        }  
        
        uid = uid;

        sb = (super_block*)malloc(sizeof(super_block));
        fread(sb,sizeof(*sb),1,disk);
        //printf("sb: %d, %d, %d, \n", sb->data_offset, sb->total_blocks, sb->fat_entries);

        fat_table = (uint16_t*)malloc(sb->fat_size);
        memset(fat_table, INITIALIZE, sb->fat_size);
        fread(fat_table,sizeof(uint16_t),sb->fat_entries,disk);
        //printf("%d\n", fat_table[0]);

        init = true;

        current_directory = NULL;
        current_directory = (dir_header *)malloc(sizeof(dir_header));
        memset((void *)current_directory, INITIALIZE, BLOCKSIZE);
        fseek(disk,sb->data_offset,SEEK_SET);
        fread(current_directory,sizeof(dir_header),1,disk);
        //printf("%d\n", current_directory->is_dir);

        assert(disk);
        assert(init);
        return SUCCESS;
    }
    return FAIL;
    
}

int create_file(char *filename, dir_header *directory){
    //get free FAT
    int idx = 1;
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
    header->unused = 0;
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

    if (!init) {
        return NULL;
    }

    if (pathname == NULL || mode < 1 || mode > 4){
        return NULL;
    }

    int fat = FAIL;
    size_t file_size = 0;
    int len = strlen(pathname);
    char *copy = strcpy( (char *)malloc(len+1), pathname);
    char *token = strtok(copy, DELIM);
    char *filename = token;
    while (token != NULL){
        filename = token;
        token = strtok(NULL, DELIM);
    } //printf("filename: %s\n", filename);

    if (filename == NULL){
        return NULL;
    }
    
    dir_header *dir = get_directory(pathname);
    if (dir == NULL){
        return NULL;
    }

    for(int i=0; i<dir->items; i++){
        if (strncmp(filename, dir->files[i].name, 12)==0 && dir->files[i].is_dir==FALSE){
            fat = dir->files[i].first_FAT;
        }
    }
    if(fat==FAIL && (mode==READ_ONLY || mode==READ_WRITE) ){
        free(filename);
        free(dir);
        return NULL;
    }
    //printf("%d\n", fat);
    if (fat==-1){
        printf("file doesnt exist\n");
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
        free(filename);
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

    return handle;
}

/*read the specified number of bytes from a file handle at the current position. 
Returns the number of bytes read, or an error.*/
size_t f_read(void *ptr, size_t size, size_t nmemb, file *stream){
    if (stream == NULL){
        return FAIL;
    }

    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (stream->curr_FAT * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(header,sizeof(file_header),1,disk);

    if (header->size == 0){
        free(header);
        return 0;
    }

    size_t total_bytes = size * nmemb;
    if (total_bytes > header->size){
        total_bytes = header->size;
    }

    char *read_data = (char *)malloc(total_bytes);
    memcpy(read_data, header->buffer + stream->read_bytes, total_bytes);

    ptr = (void *)read_data;
    
    return total_bytes;
}


/*write some bytes to a file handle at the current position. 
Returns the number of bytes written, or an error.*/
size_t f_write(void *ptr, size_t size, size_t nmemb, file *stream){

    if (stream == NULL){
        return FAIL;
    }

    uint16_t write_fat = stream->first_FAT;
    uint16_t cap = -1;
    while (fat_table[write_fat] != cap){
        write_fat = fat_table[write_fat];
    }

    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (write_fat * BLOCKSIZE);
    fseek(disk,offset,SEEK_SET);
    fread(header,sizeof(file_header),1,disk); // this is the first file block

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
        offset = sb->data_offset + (write_fat * BLOCKSIZE);
        memcpy(header->buffer + header->buffer_used, new_data, space);
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

            header->next_FAT = idx;
            
            total_blocks += 1;

            offset = sb->data_offset + (idx * BLOCKSIZE);
            memset(header, INITIALIZE, BLOCKSIZE);
            
            header->uid = uid;
            header->is_dir = FALSE;
            header->dir_FAT = dir_FAT;
            header->parent_FAT = parent_FAT;
            header->first_FAT = first_FAT;
            header->total_blocks = total_blocks;
            header->size = file_size;
            header->buffer_used = 0;
            header->unused = 0;

        }
    }
   free(header);
    
    return bytes_written;
}



/* close a file handle*/
int f_close(file *stream){
    if (stream == NULL){
        return FAIL;
    }

    free(stream);
    stream=NULL;
    return SUCCESS;
}

/*move to a specified position in a file*/
int f_seek(file *stream, long offset, int position){
    return FAIL;
}

/*move to the start of the file*/
void f_rewind(file *stream){

}

/* retrieve information about a file*/
int f_stat(file *stream, file_header *stat_buffer){
    return FAIL;
}

/*recall that directories are handled as special cases of files. 
open a “directory file” for reading, and return a directory handle.*/
dir *f_opendir(const char *name){


    return NULL;
}

/*returns a pointer to a “directory entry” structure representing 
the next directory entry in the directory file specified.*/
dir_entry *f_readdir(dir *directory){

    return NULL;
}

/* close an open directory file*/
int f_closedir(dir *stream){
    return FAIL;
}

/*make a new directory at the specified location*/
int f_mkdir(const char *pathname, char *mode){
    return FAIL;
}

/*delete a specified directory. Be sure to remove entire contents and the contents of 
all subdirectorys from the filesystem. Do NOT simply remove pointer to directory.*/
int f_rmdir(const char *pathname){
    return FAIL;
}

void f_terminate(){
    fseek(disk, BLOCKSIZE, SEEK_SET);
    fwrite(fat_table,sizeof(uint16_t),sb->fat_entries,disk);

    free(sb);
    free(fat_table);
    fclose(disk);
}

int main(){ 

    f_init(SUPERUSER, "DISK");

    file * h1 = f_open("/f1.txt", WRITE_ONLY);

    file * h2 = f_open("/f2.txt", WRITE_ONLY);

    char *buffer = "I like green apples. I like yellow apples. I like blue apples. I like purple apples. I like orange apples. I like pink apples. I like black apples. I like white apples. I like brown apples. I like grey apples. I like silver apples. I like gold apples. I like copper apples. I like bronze apples. I like brass apples. I like aluminum apples. I like iron apples.";

    char *reader = (char *)malloc(strlen(buffer));

    f_write(buffer, 1, strlen(buffer), h1);

    f_write(buffer, 1, strlen(buffer), h2);

    f_read((void *) reader, 1, strlen(buffer), h1);

    f_close(h1);

    f_close(h2);

    //f_terminate();

    return SUCCESS;
}
