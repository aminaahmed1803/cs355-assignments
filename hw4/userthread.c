#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"
#include "sjf.h"
#include "priority.h"
#include "userthread.h"

int assigned_policy;
bool init = false; 

int thread_libinit(int policy){
    if (init){
        return FAILURE;
    }

    assigned_policy = policy;
    init = true;
    switch(assigned_policy){
        case (FIFO):
            return fifo_libinit(policy);
            
        case (SJF):
            return sjf_libinit(policy);
            
        case (PRIORITY):
            return priority_libinit(policy);
            
    }
    return EXIT_FAILURE;
}

int thread_libterminate(void){
    if (!init){
        return FAILURE;
    }

    switch(assigned_policy){
        case (FIFO):
            return fifo_libterminate();
            
        case (SJF):
            return sjf_libterminate();
            
        case (PRIORITY):
            return priority_libterminate();
            
    }
    return EXIT_FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority){
    if (!init){
        return FAILURE;
    }
    if (func == NULL){
        return FAILURE;
    }

    switch(assigned_policy){
        case (FIFO):
            return fifo_create(func, arg, priority);
            
        case (SJF):
            return sjf_create(func, arg, priority);
            
        case (PRIORITY):
            return priority_create(func, arg, priority);
           
    }
    return EXIT_FAILURE;
}

int thread_yield(void){
    if (!init){
        return FAILURE;
    }

    switch(assigned_policy){
        case (FIFO):
            return fifo_yield();
            
        case (SJF):
            return sjf_yield();
            
        case (PRIORITY):
            return priority_yield();
            
    }
    return EXIT_FAILURE;

}

int thread_join(int tid){
    if (!init){
        return FAILURE;
    }
    
    switch(assigned_policy){
        case (FIFO):
            return fifo_join(tid);
           
        case (SJF):
            return sjf_join(tid);
            
        case (PRIORITY):
            return priority_join(tid);
            
    }
    init = false;
    return EXIT_FAILURE;
}
