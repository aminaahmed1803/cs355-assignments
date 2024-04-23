#ifndef __SJF_H_ 
#define __SJF_H_ "sjf_thread.h"

#include "queue.h"

int sjf_thread_libinit(int policy);

int sjf_thread_libterminate(void);

int sjf_thread_create(void (*func)(void *), void *arg, int priority);

int sjf_thread_yield(void);

int sjf_thread_join(int tid);

#endif
