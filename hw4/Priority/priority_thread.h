#ifndef __RR_H_ 
#define __RR_H_ "rr_thread.h"

#include "queue.h"


int rr_thread_libinit(int policy);

int rr_thread_create(void (*func)(void *), void *arg, int priority);

int rr_thread_yield(void);

int rr_thread_join(int tid);

int rr_thread_libterminate(void);

#endif
