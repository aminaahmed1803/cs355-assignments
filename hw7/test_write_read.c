#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fat.h" 

int read_test(){
    uid = SUPERUSER;
    f_init("DISK");

    file *h1 = f_open("/file_1", WRITE_ONLY);
    file *h2 = f_open("/file_2", WRITE_ONLY);

    char *buffer_1 = "Hello World! This is a test file. I am writing to this file using fwrite.c";
    char *buffer_2 = "I like green apples. I like yellow apples. I like blue apples. I like purple apples. I like orange apples. I like pink apples. I like black apples. I like white apples. I like brown apples. I like grey apples. I like silver apples. I like gold apples. I like copper apples. I like bronze apples. I like brass apples. I like aluminum apples. I like iron apples. I like zebra apples. I like melon apples. I like watermelon apples. I like cantaloupe apples. I like honeydew apples. I like strawberry apples. I like raspberry apples. I like blueberry apples. I like blackberry apples. I like cranberry apples. I like cherry apples. I like peach apples. I like pear apples. I like plum apples. I like grape apples. I like kiwi apples. I like pineapple apples. I like mango apples. I like papaya apples. I like banana apples. I like coconut apples. I like orange apples. I like lemon apples. I like lime apples. I like grapefruit apples. I like tangerine apples. I like clementine apples. I like mandarin apples. I like kumquat apples. I like persimmon apples. I like pomegranate apples. I like avocado apples. I like olive apples. I like tomato apples. I like cucumber apples. I like zucchini apples. I like squash apples. I like pumpkin apples. I like eggplant apples. I like pepper apples. I like onion apples. I like garlic apples. I like ginger apples. I like turmeric apples. I like cinnamon apples. I like nutmeg apples. I like clove apples. I like allspice apples. I like cumin apples. I like coriander apples. I like cardamom apples. I like mustard apples. I like horseradish apples. I like wasabi apples. I like jalapeno apples. I like habanero apples. I like ghost apples. I like scorpion apples. I like reaper apples. I like cayenne apples. I like paprika apples. I like chili apples. I like pepper apples. I like salt apples. I like sugar apples. I like honey apples. I like syrup apples. I like molasses apples. I like caramel apples. I like chocolate apples. I like vanilla apples. I like coffee apples. I like tea apples. I like milk apples. I like cream apples. I like butter apples. I like cheese apples. I like yogurt apples. I like ice cream apples. I like sorbet apples. I like sherbet apples. I like gelato apples. I like custard apples. I like pudding apples. I like jello apples. I like mousse apples. I like souffle apples. I like cake apples. I like pie apples. I like tart apples. I like c";
    
    f_write(buffer_1, 1, strlen(buffer_1), h1);
    f_write(buffer_2, 1, strlen(buffer_2), h2);

    f_close(h1);
    f_close(h2);

    f_terminate();

    return SUCCESS;

}

int write_test(){ 

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

int main() {
    read_test();
    write_test();
    return 0;
}