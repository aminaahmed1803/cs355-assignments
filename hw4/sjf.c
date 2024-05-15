#include <stdint.h>
#include <stdio.h>
#include "sjf.h"

typedef struct {
	int threads_num;
	sjf *main_thread;
	sjf *running;
	sjf *ready;
	sjf *zombie;
	sjf *waiting;
	int logger;
	
} LIB;

static LIB *library;
static bool innit = false;
static long total_runtime = DEFAULT_RUNTIME;
static long avg_runtime = DEFAULT_RUNTIME;
static struct timespec start_time; 

void sjf_enqueue(sjf **head, sjf *node){ // in acending order of t_n
   
    sjf* tmp;
    if (*head == NULL || (*head)->runtime > node->runtime) {
        node->next = *head;
        *head = node;
    }
    else {
        /* Locate the node before the point of insertion */
        tmp = *head;
        while (tmp->next != NULL && tmp->next->runtime <= node->runtime) {
            tmp = tmp->next;
        }
        node->next = tmp->next;
        tmp->next = node;
   }
}

sjf * sjf_dequeue(sjf **head){
   if (*head == NULL) {
      return NULL;
   }

   sjf *tmp = *head;
   *head = (*head)->next;
   return tmp;
}

void sjf_queue_display(sjf *head){
    sjf *tmp = head;

    while (tmp != NULL) {
    	printf("[%ld]\t%d\t%d\n", tmp->runtime, tmp->state, tmp->tid);
    	tmp = tmp->next;
    }
}

sjf *  sjf_get(sjf **head, pid_t tid ){
   if (head == NULL) {
      return NULL;
   }

   sjf *tmp = *head;
   sjf *prev = NULL;
   while (tmp != NULL) {  
      if (tmp->tid == tid) {
         if (prev != NULL) {
            prev->next = tmp->next;
         } else {
            *head = (*head)->next;;
         }
         return tmp;
      }
      prev = tmp;
      tmp = tmp->next;
    }

   return NULL;
}

sjf * sjf_return_waiting(sjf **head, pid_t waiting_tid ){
   if (head == NULL) {
      return NULL;
   }

  sjf *tmp = NULL;
  tmp = *head;
  sjf *prev = NULL;
   while (tmp != NULL) {  
      if (tmp->waiting_tid == waiting_tid) {
         if (prev != NULL) {
            prev->next = tmp->next;
         } else {
            *head = (*head)->next;;
         }
         return tmp;
      }
      prev = tmp;
      tmp = tmp->next;
    }
   return NULL;
}

bool  sjf_search(sjf *head, pid_t tid ){
   if (head == NULL) {
      return false;
   }

   sjf *tmp = head;
   while (tmp != NULL) {  
      if (tmp->tid == tid) {
         return true;
      }
      
      tmp = tmp->next;
    }

   return false;
}

void  sjf_queue_terminate(sjf **head){
   
   if (*head == NULL) {
      return;
   }

   sjf *current = *head;
   sjf *tmp;
   while (current != NULL) {
      tmp = current;
      current = current->next;
      free(tmp->context.uc_stack.ss_sp);
      free(tmp);
   }

   *head = NULL;
}

/*unsigned long  get_elapsed_time(sjf *node){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the time difference in milliseconds
   unsigned long elapsed_ms = (current_time.tv_sec - node->start_time.tv_sec) * MILLISEC;
   elapsed_ms += (current_time.tv_usec - node->start_time.tv_usec) / MILLISEC;
   return elapsed_ms;
}*/

static int logger_innit(){
	if(!innit) return FAILURE; 

	library->logger = open("userthread_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (library->logger == -1) {
        return FAILURE; 
    }
	close(library->logger);
	return EXIT_SUCCESS;
}


static void log_status(sjf *node, char *operation){
	if(!innit) return; 
	
	library->logger = open("userthread_log.txt", O_WRONLY|O_APPEND, 0644);
	if (library->logger == -1) {
        return; 
    }

	struct timespec current_time;
    if(clock_gettime(CLOCK_MONOTONIC, &current_time) == -1 ) {
      perror( "clock gettime" );
      exit( EXIT_FAILURE );
    }

	long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                        (current_time.tv_nsec - start_time.tv_nsec) / 1000;

    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "[%ld]\t%s\t%d\t-\n", elapsed_time, operation, node->tid);

    if (write(library->logger, buffer, len) != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
	close(library->logger);
}

static int sjf_thread_exit(void *return_value ){

	if (library->running == library->main_thread){
		return FAILURE; 
	}

	sjf *current = library->running;

	total_runtime += current->runtime;
	avg_runtime = total_runtime;

	//curr_thread->runtime += time_taken; 
	current->state = ZOMBIE;
	current->return_value = return_value;
	log_status(current, "FINISHED");
	sjf_enqueue(&library->zombie, current);
	
	sjf *next = sjf_return_waiting(&library->waiting, current->tid);
	while(next != NULL){
		next->state = READY;
		sjf_enqueue(&library->ready, next);
		next = sjf_return_waiting(&library->waiting, current->tid);
	}

	next = sjf_dequeue(&library->ready);

	if (next == NULL){
		library->running = library->main_thread;
		library->main_thread->state = RUNNING;
		swapcontext(&current->context, &(library->main_thread->context));
		return EXIT_SUCCESS;	
	}	// will just start the next thread

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");
	swapcontext(&current->context, &next->context);
	return EXIT_SUCCESS;
}

static void stub_function(void *(*func) (void *), void *arg) {
	sjf_thread_exit(func(arg));
}

int sjf_libinit(int policy){
	if(innit) return FAILURE; 
	innit = true; 
	
	library = (LIB*)malloc(sizeof(LIB));
	if (library == NULL){ 
		return FAILURE; 
	}

	sjf *main_thread = (sjf*)malloc(sizeof(sjf));
	if (main_thread == NULL){ 
		free(library);
		return FAILURE; 
	}

	if (logger_innit() == -1) {
		free(library);
		free(main_thread);
		return FAILURE; 
	}
	
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	main_thread->tid =  library->threads_num++;
	main_thread->start_function =  NULL;
	main_thread->args =  NULL;
	main_thread->return_value =  NULL;
	main_thread->state = RUNNING;
	main_thread->runtime = DEFAULT_RUNTIME;
	getcontext(&main_thread->context);
	
	library->main_thread = main_thread;
	library->running = main_thread;

	return EXIT_SUCCESS;
} 

int sjf_libterminate(void){
	if (library->running == NULL || library->running != library->main_thread || library->ready != NULL){
		return FAILURE;
	}

	sjf_queue_terminate(&library->main_thread);
	sjf_queue_terminate(&library->zombie);
	free(library);
	innit = false;
	return EXIT_SUCCESS;
} 

int sjf_create(void (*func)(void *), void *arg, int priority){

	if(!innit) return FAILURE; 
	
	sjf *new_thread = (sjf*)malloc(sizeof(sjf));
	if (new_thread == NULL){ 
		return FAILURE; 
	}

	new_thread->tid = library->threads_num++;
	new_thread->start_function = func;
	new_thread->args =  arg;
	new_thread->return_value =  NULL;
	new_thread->state = READY;

	getcontext(&new_thread->context);
	new_thread->context.uc_stack.ss_sp = malloc(STACKSIZE);;
	new_thread->context.uc_stack.ss_size = STACKSIZE;
	new_thread->context.uc_stack.ss_flags = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);
	log_status(new_thread, "CREATED");

	sjf_enqueue(&library->ready, new_thread);
	
	return new_thread->tid;
}

int sjf_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 
	
	sjf *curr_thread = library->running;
	
	if (curr_thread != library->main_thread){
		
	struct timespec current_time;


    	if (clock_gettime(CLOCK_MONOTONIC, &current_time) == -1) {
        	perror("clock gettime");
        	exit(EXIT_FAILURE);
   		}

    	long elapsed_time = (current_time.tv_sec - curr_thread->start_time.tv_sec) * 1000 +
                            (current_time.tv_nsec - curr_thread->start_time.tv_nsec) / 1000;
    	//int actual_runtime = elapsed_time - curr_thread->runtime;
		curr_thread->last_runtimes[0] = curr_thread->last_runtimes[1];
		curr_thread->last_runtimes[1] = curr_thread->last_runtimes[2];
		curr_thread->last_runtimes[2] = elapsed_time;

    	curr_thread->runtime = (curr_thread->last_runtimes[0] + curr_thread->last_runtimes[1] + curr_thread->last_runtimes[2]) / 3; 

		if (curr_thread->state == RUNNING) {
			curr_thread->state = READY;
		
			if (curr_thread != library->main_thread){
				sjf_enqueue(&library->ready, curr_thread);
				log_status(curr_thread, "STOPPED");
			}
		} else {
			return FAILURE;
		}
	
	}
	sjf *next_thread = sjf_dequeue(&library->ready);

	if (next_thread == NULL){
		next_thread = library->main_thread;
		next_thread->state = RUNNING;	
		library->running = next_thread;
		swapcontext(&curr_thread->context, &next_thread->context);
		return EXIT_SUCCESS;
	}

	next_thread->state = RUNNING;	
	library->running = next_thread;
	log_status(next_thread, "SCHEDULED");
	clock_gettime(CLOCK_MONOTONIC, &(next_thread->start_time));
	swapcontext(&curr_thread->context, &next_thread->context);

	return EXIT_SUCCESS;
}


int sjf_join(int tid){

	if(!innit) return FAILURE; 

	if (sjf_search(library->zombie, tid)){
		return EXIT_SUCCESS;
	}
	
	bool join = sjf_search(library->ready, tid);
	
	if (tid == 0 || library->running->tid == tid || !join){
		if (library->running == library->main_thread){ //if it does not exist but current thread is main thread call yeild 
			sjf_yield();
		}
		return FAILURE; // if it does not exist and current thread is not main thread 
	}

	if (library->running == library->main_thread){ //if it exists but current thread is main thread call yeild 
		return sjf_yield();
	}

	// if it exits and current thread is not main put current thread behind it and call yeild


	sjf *current = library->running;
	current->waiting_tid = tid;

	struct timespec current_time;
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) == -1) {
    	perror("clock gettime");
		exit(EXIT_FAILURE);
   	}

	long elapsed_time = (current_time.tv_sec - current->start_time.tv_sec) * 1000 +
                            (current_time.tv_nsec -  current->start_time.tv_nsec) / 1000;
    	//int actual_runtime = elapsed_time - curr_thread->runtime;
	current->last_runtimes[0] = current->last_runtimes[1];
	current->last_runtimes[1] = current->last_runtimes[2];
	current->last_runtimes[2] = elapsed_time;
    current->runtime = (current->last_runtimes[0] + current->last_runtimes[1] + current->last_runtimes[2]) / 3; 

	sjf_enqueue(&library->waiting, current);

	if (current->state == RUNNING) {
		library->running->state = READY;
	} else {
		return FAILURE;
	}

	log_status(current, "STOPPED"); 
	sjf *next = sjf_dequeue(&library->ready);

	if (next == NULL){
		return FAILURE;
	}

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");
	clock_gettime(CLOCK_MONOTONIC, &(next->start_time));
	swapcontext(&current->context, &next->context);

	return EXIT_SUCCESS;
}

