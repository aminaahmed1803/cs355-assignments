#include <stdio.h>
#include <stdlib.h>
#include "fifo_thread.h"
#include "rr_thread.h"
#include "sjf_thread.h"
#include "userthread.h"

int assigned_policy;

int thread_libinit(int policy){
    assigned_policy = policy;
    switch(assigned_policy){
        case (FIFO):
            return fifo_thread_libinit(policy);
            break;
        case (SJF):
            return sjf_thread_libinit(policy);
            break;
        case (PRIORITY):
            return rr_thread_libinit(policy);
            break;
    }
    return EXIT_FAILURE;
}

int thread_libterminate(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_thread_libterminate();
            break;
        case (SJF):
            return sjf_thread_libterminate();
            break;
        case (PRIORITY):
            return rr_thread_libterminate();
            break;
    }
    return EXIT_FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority){
    switch(assigned_policy){
        case (FIFO):
            return fifo_thread_create(func, arg, priority);
            break;
        case (SJF):
            return sjf_thread_create(func, arg, priority);
            break;
        case (PRIORITY):
            return rr_thread_create(func, arg, priority);
            break;
    }
    return EXIT_FAILURE;
}

int thread_yield(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_thread_yield();
            break;
        case (SJF):
            return sjf_thread_yield();
            break;
        case (PRIORITY):
            return rr_thread_yield();
            break;
    }
    return EXIT_FAILURE;

}

int thread_join(int tid){
    switch(assigned_policy){
        case (FIFO):
            return fifo_thread_join(tid);
            break;
        case (SJF):
            return sjf_thread_join(tid);
            break;
        case (PRIORITY):
            return rr_thread_join(tid);
            break;
    }
    return EXIT_FAILURE;
}
