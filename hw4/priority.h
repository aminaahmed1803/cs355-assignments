#ifndef __Priority_H_ 
#define __Priority_H_ "priority.h"

#include"common.h"

typedef struct priority_thread_control_block{
	pid_t tid;
	pid_t waiting_tid;
	ucontext_t context;
	int state;
    int p;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 
	struct timeval start_time;
    struct priority_thread_control_block *next;
} priority;


int priority_libinit(int policy);

int priority_create(void (*func)(void *), void *arg, int priority);

int priority_yield(void);

int priority_join(int tid);

int priority_libterminate(void);

#endif
