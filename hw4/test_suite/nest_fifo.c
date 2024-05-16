#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

int f1();
int f2();

int f2(){
    printf("f2\n");
    int tid = thread_create(f1, NULL, 0);
    return 0;
}

int f1(){
    printf("f1\n");
    int tid = thread_create(f2, NULL, 0);
    return 0;
}

int main(){
    thread_libinit(FIFO);
    int tid = thread_create(f1, NULL, 0);
    thread_join(tid);
    thread_libterminate();
    
    return 0;
}