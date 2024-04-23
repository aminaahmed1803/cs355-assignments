#ifndef __QUEUE_H_ 
#define __QUEUE_H_ "queue.h"

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <ucontext.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define ALPHA 0.5
#define AVERAGE_BURST 1000
#define NEG_PRIORITY -1
#define ZERO_PRIORITY 0
#define ONE_PRIORITY 1
#define NEG_WEIGHT 9
#define ZERO_WEIGHT 6
#define ONE_WEIGHT 4
#define MILLISEC 1000
#define EXIT_SUCCESS 0
#define FAILURE -1
#define STACKSIZE (256*1024)
#define INITIALIZE_VALUE 0
#define ONE_INTERVAL 300
#define ZERO_INTERVAL 200
#define NEG_INTERVAL 100
#define INTERVAL 100




enum {
	READY,
	RUNNING,
	ZOMBIE,
} STATUS;

typedef struct fifo_thread_control_block{
	pid_t tid;
	ucontext_t context;
	int state;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 	
   struct fifo_thread_control_block *next;
} FIFO_TCB;

typedef struct rr_thread_control_block{
	pid_t tid;
	ucontext_t context;
	int state;
    int priority;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 
	struct timeval start_time;
    struct rr_thread_control_block *next;
} RR_TCB;


typedef struct sjf_thread_control_block{
	pid_t tid;
	ucontext_t context;
	int state;
	void (*start_function) (void *); 	
	void *args; 				
	void *return_value; 	
    int T_n; // actual length of last CPU burst
    int t_n; // previous prediction
	struct timeval start_time;
    struct sjf_thread_control_block *next;

} SJF_TCB;

//FIFO

void fifo_enqueue(FIFO_TCB **head, FIFO_TCB *node);

void fifo_enqueue_at_head(FIFO_TCB **head, FIFO_TCB *node);

FIFO_TCB * fifo_dequeue(FIFO_TCB **head);

void fifo_queue_display(FIFO_TCB *head);

FIFO_TCB * fifo_queue_search(FIFO_TCB **head, pid_t tid );

void fifo_queue_terminate(FIFO_TCB **head);

//Shortest Job First

void sjf_enqueue(SJF_TCB **head, SJF_TCB *node);

SJF_TCB * sjf_dequeue(SJF_TCB **head);

void sjf_queue_display(SJF_TCB *head);

SJF_TCB * sjf_queue_search(SJF_TCB **head, pid_t tid );

void sjf_queue_terminate(SJF_TCB **head);

unsigned long sjf_get_elapsed_time(SJF_TCB *node);

// Priority, RR

void rr_enqueue(RR_TCB **head, RR_TCB *node);

RR_TCB * rr_dequeue(RR_TCB **head);

void rr_queue_display(RR_TCB *head);

RR_TCB * rr_queue_search(RR_TCB **head, pid_t tid );

void rr_queue_terminate(RR_TCB **head);

RR_TCB * rr_pick_priority(RR_TCB *neg_one, RR_TCB *zero, RR_TCB* one);

unsigned long rr_get_elapsed_time(RR_TCB *node);

int rr_priority_enque(RR_TCB *node, RR_TCB **ready_one, RR_TCB **ready_zero, RR_TCB **ready_neg_one);

#endif