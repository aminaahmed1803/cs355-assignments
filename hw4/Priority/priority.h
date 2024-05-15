#ifndef __Priority_H_ 
#define __Priority_H_ "priority.h"

#include"common.h"

typedef struct priority_thread_control_block{
	pid_t tid;
	ucontext_t context;
	int state;
    int priority;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 
	struct timeval start_time;
    struct rr_thread_control_block *next;
} priority;


int rr_thread_libinit(int policy);

int rr_thread_create(void (*func)(void *), void *arg, int priority);

int rr_thread_yield(void);

int rr_thread_join(int tid);

int rr_thread_libterminate(void);

#endif
