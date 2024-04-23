#include "rr_thread.h"
#include "queue.h"

typedef struct {
	int threads_num;
	RR_TCB *main_thread;
	RR_TCB *running;
	RR_TCB *ready_one;
    RR_TCB *ready_zero;
    RR_TCB *ready_neg_one;
	RR_TCB *zombie;
	int logger;
	unsigned long ticks;
} thread_library;

static sigset_t block_alarm;
static sigset_t old_set;
static struct itimerval timer; // when a timer should expire
static struct itimerval old_timer;
static struct sigaction act;
static struct sigaction old_act;
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

static void log_TCB(RR_TCB *node, char *operation){
	if(!innit) return; 

	thread_lib->logger = open("userthread_log.txt", O_WRONLY|O_APPEND, 0644);
	if (thread_lib->logger == -1) {
        return; 
    }

    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "[%ld]\t%s\t%d\t%d\n", thread_lib->ticks, operation, node->tid, node->priority);

    if (write(thread_lib->logger, buffer, len) != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
	close(thread_lib->logger);
}


void handler(int signum){
	if (signum == SIGVTALRM){
		rr_thread_yield();
	}
}


void set_timer(int interval) {

	act.sa_handler = handler; // "The action to be associated with signum"
	sigemptyset(&act.sa_mask); // initialize
	act.sa_flags = INITIALIZE_VALUE;
	timer.it_interval.tv_sec = INITIALIZE_VALUE; // Will always be below 1 full second
	timer.it_value.tv_sec = INITIALIZE_VALUE;

	sigemptyset(&block_alarm);
	sigaddset(&block_alarm, SIGVTALRM); // The alarm signal responsible for blocking.

	// To set it off 100 times a second
	timer.it_interval.tv_usec = (long int) ((1.0 * interval) * 1000);
	timer.it_value.tv_usec = (long int) ((1.0 * interval) * 1000);

	sigprocmask(SIG_SETMASK, NULL, &old_set); // Store main thread signal mask
	setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
	sigaction(SIGVTALRM, &act, &old_act);

}



static void set_priority_timer(int priority){
	switch (priority){
		case 1:
			set_timer(ONE_INTERVAL);
			break;
		case 0:
			set_timer(ZERO_INTERVAL);
			break;
		case -1:
			set_timer(NEG_INTERVAL);
			break;
		default:
			return;
	}
}

/*
 * thread_libinit()
 * ----------------
 * initializes a thread library 
 *
 * parameters: int policy 
 *
 * returns: 0 upon success, and -1 upon failure
 */
int rr_thread_libinit(int policy){
	
	if(innit) return FAILURE; 
	innit = true; 
	
	thread_lib = (thread_library*)malloc(sizeof(thread_library)); // start thread lib
	if (thread_lib == NULL){ 
		return FAILURE; 
	}

	if (logger_innit() == FAILURE) { // start_logger
		free(thread_lib);
		return FAILURE; 
	}

	set_timer(INTERVAL);

	RR_TCB *main_thread = (RR_TCB*)malloc(sizeof(RR_TCB)); // start main thread
	if (main_thread == NULL){ 
		free(thread_lib);
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

static int rr_thread_exit(void *return_value ){

	if (thread_lib->running == thread_lib->main_thread){
		return FAILURE; 
	}
	thread_lib->running->state = ZOMBIE;
	thread_lib->running->return_value = return_value;
	thread_lib->ticks += rr_get_elapsed_time(thread_lib->running);
	log_TCB(thread_lib->running, "FINISHED");
	rr_enqueue(&thread_lib->zombie, thread_lib->running); 

	return rr_thread_yield();
}

static void stub_function(void *(*func) (void *), void *arg) {
   rr_thread_exit(func(arg));
}


int rr_thread_create(void (*func)(void *), void *arg, int priority){

	if(!innit) return FAILURE; 

	sigprocmask(SIG_BLOCK, &block_alarm, NULL);

	RR_TCB *new_thread = (RR_TCB*)malloc(sizeof(RR_TCB));
	if (new_thread == NULL){ 
		return FAILURE; 
	}

	new_thread->tid = thread_lib->threads_num++;
	new_thread->start_function = func;
	new_thread->args =  arg;
	new_thread->return_value =  NULL;
	new_thread->state = READY;
	new_thread->priority = priority;
	getcontext(&new_thread->context);
	new_thread->context.uc_stack.ss_sp = malloc(STACKSIZE);;
	new_thread->context.uc_stack.ss_size = STACKSIZE;
	new_thread->context.uc_stack.ss_flags = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);

	if (rr_priority_enque(new_thread, &thread_lib->ready_one, &thread_lib->ready_zero, &thread_lib->ready_neg_one) == FAILURE){ 
		return FAILURE; 
	}

	log_TCB(new_thread, "CREATED");
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
	return new_thread->tid;
}


int rr_thread_yield(void){

	if(!innit) return FAILURE; 

	// mask signals here 
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);
	
	RR_TCB *curr_thread = thread_lib->running;
	RR_TCB *next_thread = rr_pick_priority(thread_lib->ready_neg_one, thread_lib->ready_zero, thread_lib->ready_one);

	if (next_thread == NULL){
		return EXIT_SUCCESS;
	}

	//set_timer(INITIALIZE_VALUE); // stop timer 
	// add check if this is because thread has ended, // no need to check if main/ if main thread exits - EXIT FAILIURE
	if (curr_thread->state == RUNNING){ //quantum has expire, or user called yield
		
		curr_thread->state = READY;
		if (curr_thread != thread_lib->main_thread){
			thread_lib->ticks += rr_get_elapsed_time(curr_thread);
			log_TCB(curr_thread, "STOPPED");
		
			if (curr_thread->priority < 1){
				curr_thread->priority += 1; // decrease priority if possible
			} 
			rr_priority_enque(curr_thread, &thread_lib->ready_one, &thread_lib->ready_zero, &thread_lib->ready_neg_one);
		}
	}

	next_thread->state = RUNNING;	
	thread_lib->running = next_thread;
	
	log_TCB(next_thread, "SCHEDULED");
	gettimeofday(&(next_thread->start_time), NULL);
	set_priority_timer(next_thread->priority);
	swapcontext(&curr_thread->context, &next_thread->context);

	// unmask signals here
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
	return EXIT_SUCCESS;
}

int rr_thread_join(int tid){
	if(!innit) return FAILURE; 

	if ( rr_queue_search(&thread_lib->zombie, tid) != NULL){
		return EXIT_SUCCESS;
	}
	RR_TCB *to_be_joined;
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);

	to_be_joined = rr_queue_search(&thread_lib->ready_one, tid);
	if (to_be_joined == NULL) {
		to_be_joined = rr_queue_search(&thread_lib->ready_zero, tid);
	}
	if (to_be_joined == NULL) {
		to_be_joined = rr_queue_search(&thread_lib->ready_neg_one, tid);
	}

	if (tid == 0  || to_be_joined == NULL ) {
		return FAILURE;
	}
	
	RR_TCB *curr_thread = thread_lib->running;

	//set_timer(INITIALIZE_VALUE); // stop timer 

	if (curr_thread->state == RUNNING){ //user called join
		
		curr_thread->state = READY;
		if (curr_thread != thread_lib->main_thread){
			thread_lib->ticks += rr_get_elapsed_time(curr_thread);
			log_TCB(curr_thread, "STOPPED");
		
			if (curr_thread->priority < 1){
				curr_thread->priority += 1; // decrease priority if possible
			} 
			rr_priority_enque(curr_thread, &thread_lib->ready_one, &thread_lib->ready_zero, &thread_lib->ready_neg_one);
		}
	} else {
		return FAILURE;
	}

	to_be_joined->state = RUNNING;	
	thread_lib->running = to_be_joined;
	log_TCB(to_be_joined, "SCHEDULED");
	gettimeofday(&(to_be_joined->start_time), NULL);
	set_priority_timer(to_be_joined->priority);
	swapcontext(&curr_thread->context, &to_be_joined->context);

	// unmask signals here
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
	return EXIT_SUCCESS;
}

int rr_thread_libterminate(void){

	if (thread_lib->running == NULL || thread_lib->running != thread_lib->main_thread || thread_lib->ready_one != NULL || thread_lib->ready_zero != NULL || thread_lib->ready_neg_one != NULL){
		return FAILURE;
	}

	sigprocmask(SIG_SETMASK, &old_set, NULL);
	set_timer(INITIALIZE_VALUE); // stop timer 
	rr_queue_terminate(&thread_lib->main_thread);
	rr_queue_terminate(&thread_lib->zombie);
	free(thread_lib);
	innit = false;	
	return EXIT_SUCCESS;
}

