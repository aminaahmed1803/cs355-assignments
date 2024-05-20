#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fat.h" 

/*open multiple files*/
int main() {

    uid = SUPERUSER;
    assert(f_init("DISK") == 0);

    file *h1 = f_open("/file_1", WRITE_ONLY);
    file *h2 = f_open("/file_2", WRITE_ONLY);

    file *h3 = f_open("/file_3", READ_ONLY);
    file *h4 = f_open("/file_4", READ_WRITE);

    file *h5 = f_open("/file_5", WRITE_ONLY);
    file *h6 = f_open("/file_6", WRITE_ONLY);
    file *h7 = f_open("/file_7", WRITE_ONLY);
    file *h8 = f_open("/file_8", WRITE_ONLY);
    file *h9 = f_open("/file_9", WRITE_ONLY);
    file *h10 = f_open("/file_10", WRITE_ONLY);

    file *h11 = f_open("/file_11", WRITE_ONLY);
    file *h12 = f_open("/file_12", WRITE_ONLY);
    file *h13 = f_open("/file_13", WRITE_ONLY);
    file *h14 = f_open("/file_14", WRITE_ONLY);
    file *h15 = f_open("/file_15", WRITE_ONLY);

    file *h16 = f_open("/file_16", WRITE_ONLY);
    file *h17 = f_open("/file_17", WRITE_ONLY);
    file *h18 = f_open("/file_18", WRITE_ONLY);
    file *h19 = f_open("/file_19", WRITE_ONLY);
    file *h20 = f_open("/file_20", WRITE_ONLY);


    assert(h1 != NULL); 
    assert(h2 != NULL);

    assert(h3 == NULL);
    assert(h4 == NULL);

    assert(h5 != NULL);
    assert(h6 != NULL);
    assert(h7 != NULL);
    assert(h8 != NULL);
    assert(h9 != NULL);
    assert(h10 != NULL); 
    assert(h11 != NULL);
    assert(h12 != NULL);
    assert(h13 != NULL);
    assert(h14 != NULL);
    assert(h15 != NULL);
    assert(h16 != NULL);
    assert(h17 != NULL);

    assert(h18 == NULL);
    assert(h19 == NULL);
    assert(h20 == NULL);

    f_close(h1);
    f_close(h2);
    f_close(h3);
    f_close(h4);
    f_close(h5);
    f_close(h6);
    f_close(h7);
    f_close(h8);
    f_close(h9);
    f_close(h10);
    f_close(h11);
    f_close(h12);
    f_close(h13);
    f_close(h14);
    f_close(h15);
    f_close(h16);
    f_close(h17);
    f_close(h18);
    f_close(h19);
    f_close(h20);

    return 0;
}
