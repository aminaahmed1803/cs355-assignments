#ifndef __SJF_H_ 
#define __SJF_H_ "sjf.h"

#include"common.h"

typedef struct sjf_thread_control_block{
	pid_t tid;
	ucontext_t context;
	
	int state;
	long runtime;

	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 	

    struct sjf_thread_control_block *next;

} sjf;

//Shortest Job First

int sjf_libinit(int policy);

int sjf_libterminate(void);

int sjf_create(void (*func)(void *), void *arg, int priority);

int sjf_yield(void);

int sjf_join(int tid);

#endif