#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fat.h" 

int main(){ 

    uid = SUPERUSER;
    f_init("DISK");

    file * h1 = f_open("/file_1", READ_ONLY);
    file * h2 = f_open("/file_2", READ_ONLY);
    file * h3 = f_open("/file_2", READ_ONLY);

    char *reader_1 = (char *)malloc(75);
    memset(reader_1, INITIALIZE, 75); 

    char *reader_2 = (char *)malloc(2411);
    memset(reader_2, INITIALIZE, 2411);

    char *reader_3 = (char *)malloc(1024);
    memset(reader_3, INITIALIZE, 1024);

    f_read((void *) reader_1, 1, 75, h1);
    f_read((void *) reader_2, 1, 2411, h2);
    f_read((void *) reader_3, 1, 1024, h3);

    printf("%s\n\n", reader_1);
    printf("%s\n\n", reader_2);
    printf("%s\n\n", reader_3);


    f_close(h1);
    f_close(h2);
    f_close(h3);

    f_terminate();

    return SUCCESS;
}