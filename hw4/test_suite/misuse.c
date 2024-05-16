#include <assert.h>
#include <stdio.h>
#include "userthread.h"

int main(){
    assert(thread_libterminate() == -1);
    assert(thread_libinit(FIFO) == 0);
    thread_create(NULL, NULL, 0);
    assert(thread_libinit(SJF) == -1);
    thread_create(NULL, NULL, 0);
    assert(thread_libinit(PRIORITY) == -1);
    thread_libterminate();
    assert(thread_create(NULL, NULL, 0) == -1);
    assert(thread_libterminate() == -1);

    return 0;   
}