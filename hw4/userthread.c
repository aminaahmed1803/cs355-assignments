#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"
#include "sjf.h"
#include "userthread.h"

int assigned_policy;

int thread_libinit(int policy){
    assigned_policy = policy;
    switch(assigned_policy){
        case (FIFO):
            return fifo_libinit(policy);
            
        case (SJF):
            return sjf_libinit(policy);
            
        /*case (PRIORITY):
            return rr_thread_libinit(policy);*/
            
    }
    return EXIT_FAILURE;
}

int thread_libterminate(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_libterminate();
            
        case (SJF):
            return sjf_libterminate();
            
        /*case (PRIORITY):
            return rr_thread_libterminate();*/
            
    }
    return EXIT_FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority){
    switch(assigned_policy){
        case (FIFO):
            return fifo_create(func, arg, priority);
            
        case (SJF):
            return sjf_create(func, arg, priority);
            
        /*case (PRIORITY):
            return rr_thread_create(func, arg, priority);*/
           
    }
    return EXIT_FAILURE;
}

int thread_yield(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_yield();
            
        case (SJF):
            return sjf_yield();
            
        /*case (PRIORITY):
            return rr_thread_yield();*/
            
    }
    return EXIT_FAILURE;

}

int thread_join(int tid){
    switch(assigned_policy){
        case (FIFO):
            return fifo_join(tid);
           
        case (SJF):
            return sjf_join(tid);
            
        /*case (PRIORITY):
            return rr_thread_join(tid);*/
            
    }
    return EXIT_FAILURE;
}

