#ifndef __FIFO_H_ 
#define __FIFO_H_ "fifo.h"

#include"common.h"

typedef struct fifo_thread_control_block{
	pid_t tid;
	pid_t waiting_tid;
	ucontext_t context;
	int state;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 	
    struct fifo_thread_control_block *next;
} fifo;


int fifo_libinit(int policy);

int fifo_create(void (*func)(void *), void *arg, int priority);

int fifo_yield(void);

int fifo_join(int tid);

int fifo_libterminate(void);

#endif