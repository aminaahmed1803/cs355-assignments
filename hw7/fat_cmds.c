#include "common.h"
#include "fat.h"

int disk_int(){
    disk = fopen("DISK", "r+");
    if(disk == NULL){
        printf("Error: Unable to open disk. Please use format.c to create a disk file by running ./format DISK -s 1\n");
        exiting();
    }
    fclose(disk);

    //display a log-in prompt.
    printf("Welcome to the FAT file system shell.\nLog in with user id 0 to log in as a Super User.\nLog in with user id 1 to log in as a Regular User\n");
    printf("Please enter your user id: ");
    scanf("%d", &uid);
    while(uid != 0 && uid != 1){
        printf("Invalid user id. Please enter a valid user id: ");
        scanf("%d", &uid);
    }
    
    if (uid==0){
        printf("You are logged in as a Super User\n");
    }
    else{
        printf("You are logged in as a Regular User\n");
    }

    //initialize the file system
    f_error = f_init("DISK");
    if(f_error == -1){
        printf("Error: Unable to initialize the file system\n");
        exiting();
    }   

    return SUCCESS;
}

int mkdir_shell(char **command){
    //make dir in current directory
    //if there are 15 then return error
    f_mkdir(command[1]);
    return SUCCESS;
}

int rmdir_shell(char **command){

    //if nothin is in the directory then error
    //else remove the directory
    f_rmdir(command[1]);
    return SUCCESS;
}

int cd(char **command){
    //change current directory to specified directory
    //handle cd . and ..
    int idx = -1;
    for (int i=0; i<current_directory->items; i++){    
        if (strcmp(command[1], current_directory->files[i].name) == 0){
            if (uid==SUPERUSER || (uid==USER && current_directory->files[i].uid==USER) ){
                idx = i;
                break;
            }else{
                printf("access not granted\n");
                return FAIL;
            }
        }
    }

    if (idx == -1){
        printf("directory not found\n");
        return FAIL;
    }

    dir_header *temp = (dir_header *)malloc(sizeof(dir_header));
    fseek(disk, sb->data_offset + (idx * BLOCKSIZE), SEEK_SET);
    fread(temp, sizeof(dir_header), 1, disk);

    free(current_directory);
    current_directory = NULL;
    current_directory = temp;
    
    return SUCCESS;
}


int remove_file(char **command) {
    //remove a file in current directory
    return SUCCESS;
}