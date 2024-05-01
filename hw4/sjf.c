#include <stdint.h>
#include <stdio.h>
#include "sjf.h"

typedef struct {
	int threads_num;
	sjf *main_thread;
	sjf *running;
	sjf *ready;
	sjf *zombie;

	
	int logger;
	
} LIB;

static LIB *library;
static bool innit = false;
static long avg_runtime = DEFAULT_RUNTIME;
static long last_runtimes[3] = {DEFAULT_RUNTIME, DEFAULT_RUNTIME, DEFAULT_RUNTIME};   
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

sjf *  sjf_queue_search(sjf **head, pid_t tid ){
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

void sjf_thread_exit(void *return_value ){

	if (library->running == library->main_thread){
		return; 
	}

	sjf *curr_thread = library->running;
	
	//library->total_runtime += time_taken;

	last_runtimes[0] = last_runtimes[1];
    last_runtimes[1] = last_runtimes[2];
    last_runtimes[2] = curr_thread->runtime;
    avg_runtime = (2 * avg_runtime + curr_thread->runtime) / 3;


	//curr_thread->runtime += time_taken; 
	curr_thread->state = ZOMBIE;
	curr_thread->return_value = return_value;
	log_status(curr_thread, "FINISHED");
	sjf_enqueue(&library->zombie, curr_thread);
	
	sjf *next_thread = sjf_dequeue(&library->ready);

	if (next_thread == NULL){
		next_thread = library->main_thread;
	}	// will just start the next thread

	next_thread->state = RUNNING;	
	library->running = next_thread;
	log_status(next_thread, "SCHEDULED");
	//gettimeofday(&(next_thread->start_time), NULL);
	swapcontext(&curr_thread->context, &next_thread->context);
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
	new_thread->runtime = (last_runtimes[0] + last_runtimes[1] + last_runtimes[2]) / 3;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);
	log_status(new_thread, "CREATED");

	/*if (library->running == library->main_thread){
		new_thread->state = RUNNING;
		log_status(new_thread, "SCHEDULED");
		library->running = new_thread;
		gettimeofday(&new_thread->start_time, NULL); //start timer
		swapcontext(&library->main_thread->context, &new_thread->context);
	}
	else {
		sjf_enqueue(&library->ready, new_thread);
	}*/

	sjf_enqueue(&library->ready, new_thread);
	
	return new_thread->tid;
}

int sjf_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 
	
	sjf *curr_thread = library->running;
	
	struct timespec current_time;

    if (clock_gettime(CLOCK_MONOTONIC, &current_time) == -1) {
        perror("clock gettime");
        exit(EXIT_FAILURE);
    }

    long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000 +
                            (current_time.tv_nsec - start_time.tv_nsec) / 1000;
    int actual_runtime = elapsed_time - curr_thread->runtime;

        // update runtime
    curr_thread->runtime = actual_runtime; 

	if (curr_thread->state == RUNNING) {
		curr_thread->state = READY;
		// over here if it has not finished running update Tn+1
		if (curr_thread != library->main_thread){
			
			//unsigned long time_taken = get_elapsed_time(curr_thread);
			//curr_thread->runtime += time_taken;
			//library->total_runtime += time_taken;

			sjf_enqueue(&library->ready, curr_thread);
			log_status(curr_thread, "STOPPED");
		}
	} // don't need to do anything if it is a zombie
	//sjf_queue_display(library->ready);

	sjf *next_thread = sjf_dequeue(&library->ready);
	//printf("\n\n");

	if (next_thread == NULL){
		next_thread = library->main_thread;
	}

	next_thread->state = RUNNING;	
	library->running = next_thread;
	log_status(next_thread, "SCHEDULED");
	//gettimeofday(&(next_thread->start_time), NULL);
	swapcontext(&curr_thread->context, &next_thread->context);

	return EXIT_SUCCESS;
}


int sjf_join(int tid){

	if(!innit) return FAILURE; 

	// add case where if to be joined is a zombie return success
	if ( sjf_queue_search(&library->zombie, tid) != NULL){
		return EXIT_SUCCESS;
	}
	
	sjf *to_be_joined = sjf_queue_search(&library->ready, tid);

	if (tid == 0 || library->running->tid == tid || to_be_joined == NULL ) {
		return FAILURE;
	}
	
	sjf *curr_thread = library->running;

	if (curr_thread->state == RUNNING) {
		curr_thread->state = READY;
		
		if (curr_thread != library->main_thread){
			
			//unsigned long time_taken = get_elapsed_time(curr_thread);
			//curr_thread->runtime += time_taken;
			//library->total_runtime += time_taken;

			sjf_enqueue(&library->ready, curr_thread);
			log_status(curr_thread, "STOPPED");
		}
	} // don't need to do anything if it is a zombie


	to_be_joined->state = RUNNING;
	library->running = to_be_joined;
	log_status(to_be_joined, "SCHEDULED");
	//gettimeofday(&(to_be_joined->start_time), NULL);
	swapcontext(&curr_thread->context, &to_be_joined->context);

	return EXIT_SUCCESS;
}

