#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fat.h" 

int main() {
  
    uid = SUPERUSER;

    f_init("DISK");

    f_mkdir("dir1");
    f_mkdir("dir2");
    f_mkdir("dir3");
    
    f_mkdir("dir1/dir1_1");
    f_mkdir("dir2/dir2_1");
    f_mkdir("dir3/dir3_1");

    f_rmdir("dir1/dir1_1");
    f_rmdir("dir2");

    return 0;
}   
