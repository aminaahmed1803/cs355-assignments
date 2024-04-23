#include "fifo_thread.h"
#include "queue.h"


typedef struct {
	int threads_num;
	FIFO_TCB *main_thread;
	FIFO_TCB *running;
	FIFO_TCB *ready;
	FIFO_TCB *zombie;
	int logger;
} thread_library;

static thread_library *thread_lib;
static bool innit = false;

static int logger_innit(){
	if(!innit) return FAILURE; 

	thread_lib->logger = open("userthread_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (thread_lib->logger == -1) {
        return FAILURE; 
    }
	close(thread_lib->logger);
	return EXIT_SUCCESS;
}


static void log_TCB(FIFO_TCB *node, char *operation){
	if(!innit) return; 

	thread_lib->logger = open("userthread_log.txt", O_WRONLY|O_APPEND, 0644);
	if (thread_lib->logger == -1) {
        return; 
    }

    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "[000]\t%s\t%d\t-\n", operation, node->tid);

    if (write(thread_lib->logger, buffer, len) != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
	close(thread_lib->logger);
}

/*
 * thread_exit()
 * ----------------
 *
 * parameters: string* - string to be tokenized.
 *
 * returns: pointer to tokenizer.
 */
static int thread_exit(void *return_value ){

	if (thread_lib->running == thread_lib->main_thread){
		return FAILURE; 
	}

	FIFO_TCB *curr_thread = thread_lib->running;
	if (curr_thread->state == RUNNING) {
		curr_thread->state = ZOMBIE;
		curr_thread->return_value = return_value;
		fifo_enqueue(&thread_lib->zombie, curr_thread);
	} else {
		return FAILURE; 
	} 
	log_TCB(curr_thread, "FINISHED");


	FIFO_TCB *next_thread = fifo_dequeue(&thread_lib->ready);
	if (next_thread == NULL){
		thread_lib->running = thread_lib->main_thread;
		thread_lib->main_thread->state = RUNNING;
		swapcontext(&curr_thread->context, &(thread_lib->main_thread->context));
		return EXIT_SUCCESS;	
	}
	
	next_thread->state = RUNNING;
	thread_lib->running = next_thread;   
    log_TCB(next_thread, "SCHEDULED");

	
	swapcontext(&curr_thread->context, &next_thread->context);
	return EXIT_SUCCESS;	
}

/*
 * stub_function()
 * ----------------
 *
 * parameters: string* - string to be tokenized.
 *
 * returns: pointer to tokenizer.
 */
static void stub_function(void *(*func) (void *), void *arg) {
   thread_exit(func(arg));
}

/*
 * fifo_libinit()
 * ----------------
 * initializes a thread library 
 *
 * parameters: int policy 
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_libinit(int policy){

	if(innit) return FAILURE; 
	innit = true; 
	
	thread_lib = (thread_library*)malloc(sizeof(thread_library));
	if (thread_lib == NULL){ 
		return FAILURE; 
	}

	FIFO_TCB *main_thread = (FIFO_TCB*)malloc(sizeof(FIFO_TCB));
	if (main_thread == NULL){ 
		free(thread_lib);
		return FAILURE; 
	}

	if (logger_innit() == -1) {
		free(thread_lib);
		free(main_thread);
		return FAILURE; 
	}

	main_thread->tid =  thread_lib->threads_num++;
	main_thread->start_function =  NULL;
	main_thread->args =  NULL;
	main_thread->return_value =  NULL;
	main_thread->state = RUNNING;
	getcontext(&main_thread->context);
	
	thread_lib->main_thread = main_thread;
	thread_lib->running = main_thread;

	return EXIT_SUCCESS;
}

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
int fifo_thread_create(void (*func)(void *), void *arg, int priority){

	if(!innit) return FAILURE; 
	
	FIFO_TCB *new_thread = (FIFO_TCB*)malloc(sizeof(FIFO_TCB));
	if (new_thread == NULL){ 
		return FAILURE; 
	}

	new_thread->tid = thread_lib->threads_num++;
	new_thread->start_function = func;
	new_thread->args =  arg;
	new_thread->return_value =  NULL;
	new_thread->state = READY;
	getcontext(&new_thread->context);

	new_thread->context.uc_stack.ss_sp = malloc(STACKSIZE);;
	new_thread->context.uc_stack.ss_size = STACKSIZE;
	new_thread->context.uc_stack.ss_flags = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);
	
	log_TCB(new_thread, "CREATED");
	fifo_enqueue(&thread_lib->ready, new_thread);	
	return new_thread->tid;
}

/*
 * thread_yield()
 * ----------------
 * causes the current/calling thread to yield the 
 * CPU to the next runnable thread
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 
	
	FIFO_TCB *curr_thread = thread_lib->running;
	FIFO_TCB *next_thread = fifo_dequeue(&thread_lib->ready);

	if (next_thread == NULL){
		return EXIT_SUCCESS;
	}

	if (curr_thread->state == RUNNING) {
		curr_thread->state = READY;
		if (curr_thread != thread_lib->main_thread){
			fifo_enqueue(&thread_lib->ready, curr_thread);
			log_TCB(curr_thread, "STOPPED");
		}
	} else {
		return FAILURE; 
	} 

	next_thread->state = RUNNING;	
	thread_lib->running = next_thread;
	log_TCB(next_thread, "SCHEDULED");
	swapcontext(&curr_thread->context, &next_thread->context);

	return EXIT_SUCCESS;

	// can't yeild if only one job; can't yield if main
	// main can yield to other jobs but idk if this will be usable
}

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
int fifo_thread_join(int tid){

	if(!innit) return FAILURE; 

	if ( fifo_queue_search(&thread_lib->zombie, tid) != NULL){
		return EXIT_SUCCESS;
	}
	
	// add case where if to be joined is a zombie return success
	FIFO_TCB *to_be_joined = fifo_queue_search(&thread_lib->ready, tid);

	if (tid == 0 || thread_lib->running->tid == tid || to_be_joined == NULL) {
		return FAILURE;
	}
	
	FIFO_TCB *curr_thread = thread_lib->running;
	
	if (curr_thread->state == RUNNING) {
		thread_lib->running->state = READY;
		if (curr_thread != thread_lib->main_thread){
			fifo_enqueue_at_head(&thread_lib->ready, curr_thread);
			log_TCB(curr_thread, "STOPPED");
		}
	} else {
		return FAILURE;
	}

	to_be_joined->state = RUNNING;
	thread_lib->running = to_be_joined;
	log_TCB(to_be_joined, "SCHEDULED");
	swapcontext(&curr_thread->context, &to_be_joined->context);

	return EXIT_SUCCESS;
}

/*
 * fifo_lib terminate()
 * ----------------
 * shut down the thread library 
 * frees allocated memory
 *
 * parameters: void
 *
 * returns: 0 upon success, and -1 upon failure
 */
int fifo_thread_libterminate(void){
	
	if (thread_lib->running == NULL || thread_lib->running != thread_lib->main_thread || thread_lib->ready != NULL){
		return FAILURE;
	}

	fifo_queue_terminate(&thread_lib->main_thread);
	fifo_queue_terminate(&thread_lib->zombie);
	free(thread_lib);
	innit = false;
	return EXIT_SUCCESS;
}


