#ifndef __FIFO_H_ 
#define __FIFO_H_ "fifo_thread.h"

#include "queue.h"



/*
 * thread_libinit()
 * ----------------
 * initializes a thread library 
 *
 * parameters: int policy 
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_libinit(int policy);
/*
 * thread_create()
 * ----------------
 * creates a thread 
 * 
 * parameters: func - function thread needs to execute
 * 			   arg - arguments to the function
 * 			   priority - specifies the priority level if priority scheduling is selected
 *
 * returns: tid - the thread ID upon success, else -1
 */
int fifo_thread_create(void (*func)(void *), void *arg, int priority);

/*
 * thread_yield()
 * ----------------
 * causes the current/calling thread to yield the 
 * CPU to the next runnable thread
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_yield(void);
/*
 * thread_join()
 * ----------------
 * suspends execution of the calling thread 
 * until the target thread terminates
 * 
 * parameters: tid - the pid of the target thread
 *
 * returns: returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_join(int tid);

/*
 * thread_lib terminate()
 * ----------------
 * shut down the thread library 
 * frees allocated memory
 *
 * parameters: void
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_libterminate(void);

#endif
