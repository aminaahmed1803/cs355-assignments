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

    //assert(current_directory != NULL);

    return SUCCESS;
}

/*ls lists all the files in the current or specified directory. Support flags -F and -l.*/
int list_dir(char **command){
    //go through current directoruy and list according to flag present in command
    // or of the sub dir and parents
    return SUCCESS;
}

int chmod_file(char **command){
    //get handle for specified file
    //changge permissions of file in header
    return SUCCESS;
}

int mkdir_shell(char **command){
    //make dir in current directory
    //if there are 15 then return error
    return SUCCESS;
}

int rmdir_shell(char **command){

    //if nothin is in the directory then error
    //else remove the directory
    return SUCCESS;
}

int cd(char **command){
    //change current directory to specified directory
    //handle cd . and ..
    return SUCCESS;
}

int pwd(char **command){
    //print current directory path
    return SUCCESS;
}

int cat(char **command) {

    return SUCCESS;
}

int more(char **command) {
    return SUCCESS;
}

int remove_file(char **command) {
    //remove a file in current directory
    return SUCCESS;
}