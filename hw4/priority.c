#include "priority.h"

typedef struct {
	int threads_num;
	priority *main_thread;
	priority *running;
	priority *ready_one;
    priority *ready_zero;
    priority *ready_neg_one;
	priority *zombie;
	priority *waiting;
	int logger;
	unsigned long ticks;
} LIB;

LIB *library;
static bool innit = false;
struct itimerval timer;
int i;

void sigalarm_handler(int signum) {
	//i+=1;
	//printf("SIGALARM %d\n", i);
	library->ticks += 100;
	priority_yield();
}

void  enqueue(priority **head, priority *node){
   
    if (*head == NULL) {
    	node->next = NULL;
    	*head = node;
    	return;
   	}

    priority *tmp = *head;
   	while (tmp->next != NULL && tmp->next->tid < node->tid) {
    	tmp = tmp->next;
   	}

   tmp->next = node;
   node->next = NULL;
}

void enqueue_at_head(priority **head, priority *node){
  priority *tmp = *head;
   *head = node;
   node->next = tmp;
}

priority * dequeue(priority **head){
   if (*head == NULL) {
      return NULL;
   }

  priority *tmp = *head;
   *head = (*head)->next;
   return tmp;
}

void display(priority *head){
  priority *tmp = head;

   while (tmp != NULL) {
      printf("[%d]\t%d\t%d\n", tmp->p, tmp->state, tmp->tid);
      tmp = tmp->next;
   }
}

bool search(priority *head, pid_t tid ){
   	if (head == NULL) {
    	return false;
   	}

  	priority *tmp = NULL;
  	tmp = head;
   	while (tmp != NULL) {  
      
		if (tmp->tid == tid) {
			return true;
      	}
      	
		tmp = tmp->next;
    }

   return false;
}

priority * return_waiting(priority **head, pid_t waiting_tid ){
   if (head == NULL) {
      return NULL;
   }

  priority *tmp = NULL;
  tmp = *head;
  priority *prev = NULL;
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

priority * return_tid(priority **head, pid_t tid ){
   if (head == NULL) {
      return NULL;
   }

  priority *tmp = NULL;
  tmp = *head;
  priority *prev = NULL;
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

void terminate(priority **head){
   
   if (*head == NULL) {
      return;
   }

  priority *current = *head;
  priority *tmp;
   while (current != NULL) {
      tmp = current;
      current = current->next;

      free(tmp->context.uc_stack.ss_sp);
	  tmp->context.uc_stack.ss_sp=NULL;
	  
      free(tmp);
	  tmp = NULL;
   }

   *head = NULL;
}

static int logger_innit(){
	if(!innit) return FAILURE; 

	library->logger = open("userthread_log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (library->logger == -1) {
        return FAILURE; 
    }
	close(library->logger);
	return EXIT_SUCCESS;
}

static void log_status(priority *node, char *operation){
	if(!innit) return; 
	
	library->logger = open("userthread_log.txt", O_WRONLY|O_APPEND, 0644);
	if (library->logger == -1) {
        return; 
    }
    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "[%ld]\t%s\t%d\t%d\n", library->ticks, operation, node->tid, node->p);

    if (write(library->logger, buffer, len) != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
	close(library->logger);

}



priority * pick_priority(){

	//threads scheduled with level -1 should run 1.5 times more often as jobs scheduled with priority level 0, 
	// which run 1.5 times more often as jobs scheduled with priority level 1.
	int weights[3];
	int total_sum = 0;
	if (library->ready_neg_one == NULL) total_sum += 0;
	else total_sum += NEG_WEIGHT;
	weights[0] = total_sum;
	if (library->ready_zero == NULL) total_sum += 0;
	else total_sum += ZERO_WEIGHT;
	weights[1] = total_sum;
	if (library->ready_one == NULL) total_sum += 0;
	else total_sum += ONE_WEIGHT;
	weights[2] = total_sum;
	if (total_sum == 0) return NULL;
	float randNum = (float) rand() / RAND_MAX;
	float target = randNum * (total_sum);
	if (target < weights[0]) {
		return dequeue(&(library->ready_neg_one));
	}
	else if (target < weights[1]) {
		return dequeue(&(library->ready_zero));
	}
	else if (target < weights[2]) {
		return dequeue(&(library->ready_one));
	}
	return NULL;

	/*int weights[3];
   int total_sum = 0;

   if (library->ready_neg_one == NULL) total_sum += 0; 
   else total_sum += NEG_WEIGHT;
   weights[0] = total_sum;

   if (library->ready_zero == NULL) total_sum += 0; 
   else total_sum += ZERO_WEIGHT;
   weights[1] = total_sum;

   if (library->ready_one == NULL) total_sum += 0; 
   else total_sum += ONE_WEIGHT;
   weights[2] = total_sum;

   if (total_sum == 0 ) return NULL;

   float randNum = (float) rand() / RAND_MAX;
   float target =  randNum * (total_sum);


   if (target < weights[0]){
      return dequeue(&(library->ready_neg_one));
   }
   else if (target < weights[1]){
    
      return dequeue(&(library->ready_zero));
   }
   else if (target < weights[2]){
      return dequeue(&(library->ready_one));
   }
   return NULL;*/
}

static int thread_exit(void *return_value){

	if (library->running == library->main_thread){
		return FAILURE; 
	}

	//stop timer from sending SIGALARM
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	priority *current = library->running;
	if (current->state == RUNNING) {
		current->state = ZOMBIE;
		current->return_value = return_value;
		enqueue(&library->zombie, current);
	} else {
		return FAILURE; 
	} 
	log_status(current, "FINISHED");

	priority *next = return_waiting(&library->waiting, current->tid);

	while(next != NULL){
		if (next->p == -1){
			enqueue(&library->ready_neg_one, next);
		}
		else if (next->p == 0){
			enqueue(&library->ready_zero, next);
		}
		else if (next->p == 1){
			enqueue(&library->ready_one, next);
		}
		next = return_waiting(&library->waiting, current->tid);
	}

	next = pick_priority();

	if (next == NULL){
		library->running = library->main_thread;
		library->main_thread->state = RUNNING;
		swapcontext(&current->context, &(library->main_thread->context));
		return EXIT_SUCCESS;	
	}
	
	next->state = RUNNING;
	library->running = next;   
    log_status(next, "SCHEDULED");

	//set a time that sends a SIGALARM after 100 milliseconds once
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	swapcontext(&current->context, &next->context);
	return EXIT_SUCCESS;	
}

static void stub_function(void *(*func) (void *), void *arg) {
   thread_exit(func(arg));
}

int priority_libinit(int policy){

	if(innit) return FAILURE; 
	innit = true; 
	
	library = (LIB*)malloc(sizeof(LIB));
	if (library == NULL){ 
		return FAILURE; 
	}

	priority *main_thread = (priority *)malloc(sizeof(priority));
	if (main_thread == NULL){ 
		free(library);
		library=NULL;
		return FAILURE; 
	}

	if (logger_innit() == -1) {
		free(library);
		free(main_thread);
		library=NULL;
		main_thread=NULL;
		return FAILURE; 
	}

	signal(SIGALRM, sigalarm_handler);

	//set a time that sends a SIGALARM after 100 milliseconds once
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	main_thread->tid =  library->threads_num++;
	main_thread->start_function =  NULL;
	main_thread->args =  NULL;
	main_thread->return_value =  NULL;
	main_thread->state = RUNNING;
	getcontext(&main_thread->context);
	
	library->main_thread = main_thread;
	library->running = library->main_thread;
	library->waiting = NULL;
	library->ready_one = NULL;
	library->ready_neg_one = NULL;
	library->ready_zero = NULL;
	library->zombie = NULL;
	library->ticks = 0;

	return EXIT_SUCCESS;
}

int priority_create(void (*func)(void *), void *arg, int p){

	if(!innit) return FAILURE; 
	
	if (p != -1 && p != 0 && p != 1){
		return FAILURE;
	}

	priority *new_thread = (priority *)malloc(sizeof(priority));
	if (new_thread == NULL){ 
		return FAILURE; 
	}

	new_thread->tid = library->threads_num++;
	new_thread->waiting_tid = -1;
	new_thread->start_function = func;
	new_thread->args =  arg;
	new_thread->return_value =  NULL;
	new_thread->p = p;
	new_thread->state = READY;
	getcontext(&new_thread->context);

	new_thread->context.uc_stack.ss_sp = malloc(STACKSIZE);;
	new_thread->context.uc_stack.ss_size = STACKSIZE;
	new_thread->context.uc_stack.ss_flags = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);

	log_status(new_thread, "CREATED");

	if (p == 1){
		enqueue(&library->ready_one, new_thread);
		new_thread->p = 1;
	}
	else if (p == 0){
		enqueue(&library->ready_zero, new_thread);
		new_thread->p = 0;
	}
	else if (p == -1){
		enqueue(&library->ready_neg_one, new_thread);
		new_thread->p = -1;
	}
	else{
		return FAILURE;
	}


	return new_thread->tid;
}

int priority_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 

	//stop timer from sending SIGALARM
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	
	priority *current = library->running;

	if (current->state == RUNNING) {
		current->state = READY;
		if (current != library->main_thread){
			
			if (current->p == -1){
				enqueue(&library->ready_neg_one, current);
			}
			else if (current->p == 0){
				enqueue(&library->ready_zero, current);
			}
			else if (current->p == 1){
				enqueue(&library->ready_one, current);
			}
			else{
				return FAILURE;
			}
			
			log_status(current, "STOPPED");
		}
	} else {
		return FAILURE; 
	} 

	priority *next = pick_priority();
	
	if (next == NULL){
		//set a time that sends a SIGALARM after 100 milliseconds once
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 100000;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);

		return EXIT_SUCCESS;
	}

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");

	//set a time that sends a SIGALARM after 100 milliseconds once
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	swapcontext(&current->context, &next->context);

	return EXIT_SUCCESS;

}

int priority_join(int tid){

	if(!innit) return FAILURE; 
	
	//stop timer from sending SIGALARM
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	if (search(library->zombie, tid)){
		return EXIT_SUCCESS;
	}
	
	bool join = search(library->ready_one, tid) ||  search(library->ready_neg_one, tid) ||  search(library->ready_zero, tid);
	

	if (tid == 0 || library->running->tid == tid || !join){
		if (library->running == library->main_thread){ //if it does not exist but current thread is main thread call yeild 
			priority_yield();
		}
		return FAILURE; // if it does not exist and current thread is not main thread 
	}

	if (library->running == library->main_thread){ //if it exists but current thread is main thread call yeild 
		return priority_yield();
	}

	// if it exits and current thread is not main put current thread behind it and call yeild

	priority *current = library->running;
	current->waiting_tid = tid;
	enqueue(&library->waiting, current);

	if (current->state == RUNNING) {
		library->running->state = READY;
	} else {
		return FAILURE;
	}
	log_status(current, "STOPPED"); 

	priority *next = pick_priority();
	
	if (next == NULL){
		return FAILURE;
	}

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");

	//set a time that sends a SIGALARM after 100 milliseconds once
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	
	swapcontext(&current->context, &next->context);

	return EXIT_SUCCESS;
}

int priority_libterminate(void){
	
	if(!innit) return FAILURE; 
	
	if (library->running == NULL || library->running != library->main_thread || library->ready_neg_one != NULL || library->ready_zero != NULL || library->ready_one != NULL || library->waiting){
		return FAILURE;
	}

	//fifo_terminate(&library->main_thread);
	free(library->main_thread);
	library->main_thread=NULL;

	terminate(&library->zombie);
	library->zombie=NULL;

	free(library);
	library=NULL;

	innit = false;
	return EXIT_SUCCESS;
}
