#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fat.h" 

int main(){ 

    uid = SUPERUSER;
    f_init("DISK");

    f_remove("/file_1");
    f_remove("/file_2");  

    f_terminate();

    return SUCCESS;
}