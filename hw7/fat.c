#include <stdio.h>
#include "fat.h"

//have a array of file handles
//have a array of directory handles

int f_init(char *filename){
    if (!fat_init){
    
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

        current_directory = NULL;
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
            //need to add UID here
        }
    }
    if(fat==FAIL && (mode==READ_ONLY || mode==READ_WRITE) ){
        free(filename);
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
    if (!fat_init){
        return FAIL;
    }

    if (stream == NULL){
        return FAIL;
    }

    file_header *header = (file_header *)malloc(sizeof(file_header));
    memset(header, INITIALIZE, BLOCKSIZE);
    int offset = sb->data_offset + (stream->curr_FAT * BLOCKSIZE);
    

    size_t total_bytes = size * nmemb;
    size_t bytes_read = 0;
    uint16_t idx = stream->curr_FAT;

    char *read_buffer = (char *)ptr;

    do {

        fseek(disk,offset,SEEK_SET);
        fread(header,sizeof(file_header),1,disk);

        int already_read = stream->read_bytes;
        int to_read = header->buffer_used - already_read;
        memcpy(read_buffer, header->buffer + already_read, to_read);

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
    
    
    free(header);
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
            header->unused = 0;
            header->next_FAT = -1;

        }
    }
    header->total_blocks = total_blocks;
    header->size = file_size + total_bytes;

    sanity_check(header);
    stream->curr_FAT = write_fat;
    free(header);
    header = NULL;
    
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

    return FAIL;
}

/*move to the start of the file*/
void f_rewind(file *stream){


    if (!fat_init){
        return;
    }
    

}

/* retrieve information about a file*/
int f_stat(file *stream, file_header *stat_buffer){


    if (!fat_init){
        return FAIL;
    }

    return FAIL;
}

/*recall that directories are handled as special cases of files. 
open a “directory file” for reading, and return a directory handle.*/
dir *f_opendir(char *name){

    if (!fat_init){
        return NULL;
    }

    dir *handle = (dir *)malloc(sizeof(dir));
    memset(handle, INITIALIZE, sizeof(dir));

    for(int i=0; i<current_directory->items; i++){
        if (strncmp(name, current_directory->files[i].name, 12)==0 && current_directory->files[i].is_dir==TRUE){
            if (uid == current_directory->files[i].uid || uid == SUPERUSER){
                
                dir_header *directory = (dir_header *)malloc(sizeof(dir_header));
                memset((void *)directory, INITIALIZE, BLOCKSIZE);
                int offset = sb->data_offset + (current_directory->files[i].first_FAT * BLOCKSIZE);
                fseek(disk,offset,SEEK_SET);
                fread(directory,sizeof(dir_header),1,disk);

                strncpy(handle->name, name, 12);
                handle->uid = uid;
                handle->items = directory->items;
                handle->first_FAT = directory->first_FAT;
                handle->idx = 0;
                memcpy(handle->current, &(directory->files[handle->idx]), sizeof(dir_entry));


                free(directory);
                directory = NULL;

                return  handle;
            }
            else{
                free(handle);
                return NULL;
            }
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
    return NULL;
}

/* close an open directory file*/
int f_closedir(dir *stream){

    if (!fat_init){
        return FAIL;
    }
    return FAIL;
}

/*make a new directory at the specified location*/
int f_mkdir(char *pathname, char *mode){

    if (!fat_init){
        return FAIL;
    }
    return FAIL;
}

/*delete a specified directory. Be sure to remove entire contents and the contents of 
all subdirectorys from the filesystem. Do NOT simply remove pointer to directory.*/
int f_rmdir(char *pathname){
    return FAIL;
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


test_fopen1(){

    f_init("DISK");
    file * h1 = f_open("/f1.txt", WRITE_ONLY);
    file * h2 = f_open("/f2.txt", WRITE_ONLY);
    file * h3 = f_open("/f3.txt", WRITE_ONLY);
    file * h4 = f_open("/f4.txt", WRITE_ONLY);
    file * h5 = f_open("/f5.txt", WRITE_ONLY);
    file * h6 = f_open("/f6.txt", WRITE_ONLY);
    file * h7 = f_open("/f7.txt", WRITE_ONLY);
    file * h8 = f_open("/f8.txt", WRITE_ONLY);
    file * h9 = f_open("/f9.txt", WRITE_ONLY);
    file * h10 = f_open("/f10.txt", WRITE_ONLY);
    file * h11 = f_open("/f11.txt", WRITE_ONLY);
    file * h12 = f_open("/f12.txt", WRITE_ONLY);
    file * h13 = f_open("/f13.txt", WRITE_ONLY);
    file * h14 = f_open("/f14.txt", WRITE_ONLY);
    file * h15 = f_open("/f15.txt", WRITE_ONLY);

    assert(f_open("/f15.txt", WRITE_ONLY) == NULL );

}

test_fopen(){

    f_init("DISK");
    file * h1 = f_open("/f1.txt", READ_ONLY);
    file * h2 = f_open("/f2.txt", WRITE_ONLY);
    file * h3 = f_open("/f3.txt", WRITE_ONLY);

    assert(h1 == NULL );
    assert(h2 != NULL );
    assert(h3 != NULL );

}

int main(){ 

    uid = SUPERUSER;

    f_init("DISK");

    file * h1 = f_open("/f1.txt", WRITE_ONLY);

    //file * h2 = f_open("/f2.txt", WRITE_ONLY);


    char *buffer = "I like green apples. I like yellow apples. I like blue apples. I like purple apples. I like orange apples. I like pink apples. I like black apples. I like white apples. I like brown apples. I like grey apples. I like silver apples. I like gold apples. I like copper apples. I like bronze apples. I like brass apples. I like aluminum apples. I like iron apples. I like zebra apples. I like melon apples. I like watermelon apples. I like cantaloupe apples. I like honeydew apples. I like strawberry apples. I like raspberry apples. I like blueberry apples. I like blackberry apples. I like cranberry apples. I like cherry apples. I like peach apples. I like pear apples. I like plum apples. I like grape apples. I like kiwi apples. I like pineapple apples. I like mango apples. I like papaya apples. I like banana apples. I like coconut apples. I like orange apples. I like lemon apples. I like lime apples. I like grapefruit apples. I like tangerine apples. I like clementine apples. I like mandarin apples. I like kumquat apples. I like persimmon apples. I like pomegranate apples. I like avocado apples. I like olive apples. I like tomato apples. I like cucumber apples. I like zucchini apples. I like squash apples. I like pumpkin apples. I like eggplant apples. I like pepper apples. I like onion apples. I like garlic apples. I like ginger apples. I like turmeric apples. I like cinnamon apples. I like nutmeg apples. I like clove apples. I like allspice apples. I like cumin apples. I like coriander apples. I like cardamom apples. I like mustard apples. I like horseradish apples. I like wasabi apples. I like jalapeno apples. I like habanero apples. I like ghost apples. I like scorpion apples. I like reaper apples. I like cayenne apples. I like paprika apples. I like chili apples. I like pepper apples. I like salt apples. I like sugar apples. I like honey apples. I like syrup apples. I like molasses apples. I like caramel apples. I like chocolate apples. I like vanilla apples. I like coffee apples. I like tea apples. I like milk apples. I like cream apples. I like butter apples. I like cheese apples. I like yogurt apples. I like ice cream apples. I like sorbet apples. I like sherbet apples. I like gelato apples. I like custard apples. I like pudding apples. I like jello apples. I like mousse apples. I like souffle apples. I like cake apples. I like pie apples. I like tart apples. I like c";

    char *reader = (char *)malloc(strlen(buffer));
    memset(reader, INITIALIZE, strlen(buffer)); 

    f_write(buffer, 1, strlen(buffer), h1);

    //f_write(buffer, 1, strlen(buffer), h2);

    //f_read((void *) reader, 1, 510, h1);


    printf("%s\n", reader);

    f_close(h1);

    //f_close(h2);

    f_terminate();

    return SUCCESS;
}

