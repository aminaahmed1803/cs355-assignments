#include "sjf_thread.h"
#include "queue.h"

typedef struct {
	int threads_num;
	SJF_TCB *main_thread;
	SJF_TCB *running;
	SJF_TCB *ready;
	SJF_TCB *zombie;
	int logger;
	sigset_t block_sig_set;
	sigset_t  main_sig_set;
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


static void log_TCB(SJF_TCB *node, char *operation){
	if(!innit) return; 
	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);

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
	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);
}

void sjf_set_timer(int interval) {
    struct itimerval it_val;

    // Configure timer to expire after "milliseconds" milliseconds
    it_val.it_value.tv_sec = interval / 1000;
    it_val.it_value.tv_usec = (interval % 1000) * 1000;
    
    // Set the interval to expire every "milliseconds" milliseconds
    it_val.it_interval.tv_sec = interval / 1000;
    it_val.it_interval.tv_usec = (interval % 1000) * 1000;

    // Set timer
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }
}


static void handler(int signum){
	if (signum == SIGALRM){
		sjf_thread_yield();
	}
}


int sjf_thread_exit(void *return_value ){

	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);

	if (thread_lib->running == thread_lib->main_thread){
		return FAILURE; 
	}

	SJF_TCB *curr_thread = thread_lib->running;
	curr_thread->state = ZOMBIE;
	curr_thread->return_value = return_value;
	log_TCB(curr_thread, "FINISHED");
	sjf_enqueue(&thread_lib->zombie, curr_thread);

	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);
	
	return sjf_thread_yield();	// will just start the next thread
}


static void stub_function(void *(*func) (void *), void *arg) {
	sjf_thread_exit(func(arg));
}

int sjf_thread_libinit(int policy){
	if(innit) return FAILURE; 
	innit = true; 
	
	thread_lib = (thread_library*)malloc(sizeof(thread_library));
	if (thread_lib == NULL){ 
		return FAILURE; 
	}

	SJF_TCB *main_thread = (SJF_TCB*)malloc(sizeof(SJF_TCB));
	if (main_thread == NULL){ 
		free(thread_lib);
		return FAILURE; 
	}

	if (logger_innit() == -1) {
		free(thread_lib);
		free(main_thread);
		return FAILURE; 
	}

	signal(SIGALRM, handler); //set signal handler

	sigemptyset(&(thread_lib->block_sig_set)); // set masks 
	sigaddset(&(thread_lib->block_sig_set), SIGALRM);
	sigprocmask(SIG_SETMASK, NULL, &(thread_lib->main_sig_set)); // Store main thread signal mask

	main_thread->tid =  thread_lib->threads_num++;
	main_thread->start_function =  NULL;
	main_thread->args =  NULL;
	main_thread->return_value =  NULL;
	main_thread->state = RUNNING;
	main_thread->T_n = INT_MAX;
	getcontext(&main_thread->context);
	
	thread_lib->main_thread = main_thread;
	thread_lib->running = main_thread;

	return EXIT_SUCCESS;
} 

int sjf_thread_libterminate(void){
	if (thread_lib->running == NULL || thread_lib->running != thread_lib->main_thread || thread_lib->ready != NULL){
		return FAILURE;
	}

	sigprocmask(SIG_SETMASK, &(thread_lib->main_sig_set), NULL);
	sjf_set_timer(INITIALIZE_VALUE); // stop timer 
	sjf_queue_terminate(&thread_lib->main_thread);
	sjf_queue_terminate(&thread_lib->zombie);
	free(thread_lib);
	innit = false;
	return EXIT_SUCCESS;
} 

static void update_burst_prediction(SJF_TCB *node){
	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);

	node->T_n =sjf_get_elapsed_time(node);
	node->t_n = ALPHA*(node->T_n) + (ALPHA)*(node->t_n);

	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);
}

int sjf_thread_create(void (*func)(void *), void *arg, int priority){

	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);

	if(!innit) return FAILURE; 
	
	SJF_TCB *new_thread = (SJF_TCB*)malloc(sizeof(SJF_TCB));
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
	new_thread->t_n = AVERAGE_BURST;
	new_thread->T_n = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);
	log_TCB(new_thread, "CREATED");
	sjf_enqueue(&thread_lib->ready, new_thread);

	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);

	return new_thread->tid;
}

int sjf_thread_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 

	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);
	
	SJF_TCB *curr_thread = thread_lib->running;
	SJF_TCB *next_thread = sjf_dequeue(&thread_lib->ready);

	if (next_thread == NULL){
		return EXIT_SUCCESS;
	}

	if (curr_thread->state == RUNNING) {
		curr_thread->state = READY;
		// over here if it has not finished running update Tn+1
		if (curr_thread != thread_lib->main_thread){
			update_burst_prediction(curr_thread);
			sjf_enqueue(&thread_lib->ready, curr_thread);
			log_TCB(curr_thread, "STOPPED");
		}
	} // don't need to do anything if it is a zombie

	next_thread->state = RUNNING;	
	thread_lib->running = next_thread;
	log_TCB(next_thread, "SCHEDULED");
	gettimeofday(&(next_thread->start_time), NULL);
	sjf_set_timer(next_thread->t_n * 2);
	swapcontext(&curr_thread->context, &next_thread->context);

	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);


	return EXIT_SUCCESS;

}


int sjf_thread_join(int tid){

	if(!innit) return FAILURE; 

	if ( sjf_queue_search(&thread_lib->zombie, tid) != NULL){
		return EXIT_SUCCESS;
	}

	sigprocmask(SIG_BLOCK, &(thread_lib->block_sig_set), NULL);

	// add case where if to be joined is a zombie return success
	SJF_TCB *to_be_joined = sjf_queue_search(&thread_lib->ready, tid);

	if (tid == 0 || thread_lib->running->tid == tid || to_be_joined == NULL ) {
		return FAILURE;
	}
	
	SJF_TCB *curr_thread = thread_lib->running;

	if (curr_thread->state == RUNNING) {
		curr_thread->state = READY;
		if (curr_thread != thread_lib->main_thread){
			update_burst_prediction(curr_thread);
			sjf_enqueue(&thread_lib->ready, curr_thread);
			log_TCB(curr_thread, "STOPPED");
		}
	} // don't need to do anything if it is a zombie
	
 	else {
		return FAILURE;
	}

	to_be_joined->state = RUNNING;
	thread_lib->running = to_be_joined;
	log_TCB(to_be_joined, "SCHEDULED");
	gettimeofday(&(to_be_joined->start_time), NULL);
	sjf_set_timer(to_be_joined->t_n * 2);
	swapcontext(&curr_thread->context, &to_be_joined->context);

	sigprocmask(SIG_UNBLOCK, &(thread_lib->block_sig_set), NULL);

	return EXIT_SUCCESS;
}