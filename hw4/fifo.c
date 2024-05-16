#include "fifo.h"

typedef struct {
	int threads_num;
	fifo *main_thread;
	fifo *running;
	fifo *ready;
	fifo *zombie;
	fifo *waiting;
	int logger;
} LIB;

LIB *library;
static bool innit = false;


void  fifo_enqueue(fifo **head, fifo *node){
   
    if (*head == NULL) {
    	node->next = NULL;
    	*head = node;
    	return;
   	}

    fifo *tmp = *head;
   	while (tmp->next != NULL && tmp->next->tid < node->tid) {
    	tmp = tmp->next;
   	}

   tmp->next = node;
   node->next = NULL;
}


void fifo_enqueue_at_head(fifo **head, fifo *node){
  fifo *tmp = *head;
   *head = node;
   node->next = tmp;
}


fifo * fifo_dequeue(fifo **head){
   if (*head == NULL) {
      return NULL;
   }

  fifo *tmp = *head;
   *head = (*head)->next;
   return tmp;
}


void fifo_display(fifo *head){
  fifo *tmp = head;

   while (tmp != NULL) {
      printf("[%d]\t%d\t%d\n", 0, tmp->state, tmp->tid);
      tmp = tmp->next;
   }
}

bool fifo_search(fifo *head, pid_t tid ){
   	if (head == NULL) {
    	return false;
   	}

  	fifo *tmp = NULL;
  	tmp = head;
   	while (tmp != NULL) {  
      
		if (tmp->tid == tid) {
			return true;
      	}
      	
		tmp = tmp->next;
    }

   return false;
}

fifo * fifo_return_waiting(fifo **head, pid_t waiting_tid ){
   if (head == NULL) {
      return NULL;
   }

  fifo *tmp = NULL;
  tmp = *head;
  fifo *prev = NULL;
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

fifo * fifo_return(fifo **head, pid_t tid ){
   if (head == NULL) {
      return NULL;
   }

  fifo *tmp = NULL;
  tmp = *head;
  fifo *prev = NULL;
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


void fifo_terminate(fifo **head){
   
   if (*head == NULL) {
      return;
   }

  fifo *current = *head;
  fifo *tmp;
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


static void log_status(fifo *node, char *operation){
	if(!innit) return; 

	library->logger = open("userthread_log.txt", O_WRONLY|O_APPEND, 0644);
	if (library->logger == -1) {
        return; 
    }

    char buffer[1024];
	memset(buffer,INITIALIZE_VALUE,1024);
    int len = snprintf(buffer, sizeof(buffer), "[000]\t%s\t%d\t-\n", operation, node->tid);

    if (write(library->logger, buffer, len) != len) {
        perror("Error writing to file");
        exit(EXIT_FAILURE);
    }
	close(library->logger);
}


static int thread_exit(void *return_value){

	if (library->running == library->main_thread){
		return FAILURE; 
	}

	fifo *current = library->running;
	if (current->state == RUNNING) {
		current->state = ZOMBIE;
		current->return_value = return_value;
		fifo_enqueue(&library->zombie, current);
	} else {
		return FAILURE; 
	} 
	log_status(current, "FINISHED");

	fifo *next = fifo_return_waiting(&library->waiting, current->tid);

	if (next == NULL) next = fifo_dequeue(&library->ready);

	if (next == NULL){
		library->running = library->main_thread;
		library->main_thread->state = RUNNING;
		swapcontext(&current->context, &(library->main_thread->context));
		return EXIT_SUCCESS;	
	}
	
	next->state = RUNNING;
	library->running = next;   
    log_status(next, "SCHEDULED");

	swapcontext(&current->context, &next->context);
	return EXIT_SUCCESS;	
}


static void stub_function(void *(*func) (void *), void *arg) {
   thread_exit(func(arg));
}

int fifo_libinit(int policy){

	if(innit) return FAILURE; 
	innit = true; 
	
	library = (LIB*)malloc(sizeof(LIB));
	if (library == NULL){ 
		return FAILURE; 
	}

	fifo *main_thread = (fifo*)malloc(sizeof(fifo));
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

	main_thread->tid =  library->threads_num++;
	main_thread->start_function =  NULL;
	main_thread->args =  NULL;
	main_thread->return_value =  NULL;
	main_thread->state = RUNNING;
	getcontext(&main_thread->context);
	
	library->main_thread = main_thread;
	library->running = library->main_thread;
	library->waiting = NULL;

	library->ready = NULL;
	library->zombie = NULL;

	return EXIT_SUCCESS;
}

int fifo_create(void (*func)(void *), void *arg, int priority){

	if(!innit) return FAILURE; 
	
	fifo *new_thread = (fifo*)malloc(sizeof(fifo));
	if (new_thread == NULL){ 
		return FAILURE; 
	}

	new_thread->tid = library->threads_num++;
	new_thread->waiting_tid = -1;
	new_thread->start_function = func;
	new_thread->args =  arg;
	new_thread->return_value =  NULL;
	new_thread->state = READY;
	getcontext(&new_thread->context);

	new_thread->context.uc_stack.ss_sp = malloc(STACKSIZE);;
	new_thread->context.uc_stack.ss_size = STACKSIZE;
	new_thread->context.uc_stack.ss_flags = 0;
	makecontext(&new_thread->context, (void (*) (void)) stub_function, 2, func, arg);


	// if there is no thread scheduled, then schedule this thread
	log_status(new_thread, "CREATED");

	fifo_enqueue(&library->ready, new_thread);	
	return new_thread->tid;
}

int fifo_yield(void){
	// Add running process back to the ready queue.
	if(!innit) return FAILURE; 
	
	fifo *current = library->running;
	fifo *next = fifo_dequeue(&library->ready);

	if (next == NULL){
		return EXIT_SUCCESS;
	}

	if (current->state == RUNNING) {
		current->state = READY;
		if (current != library->main_thread){
			fifo_enqueue(&library->ready, current);
			log_status(current, "STOPPED");
		}
	} else {
		return FAILURE; 
	} 

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");
	swapcontext(&current->context, &next->context);

	return EXIT_SUCCESS;

}

int fifo_join(int tid){

	if(!innit) return FAILURE; 
	

	if (fifo_search(library->zombie, tid)){
		return EXIT_SUCCESS;
	}
	
	bool join = fifo_search(library->ready, tid);
	

	if (tid == 0 || library->running->tid == tid || !join){
		if (library->running == library->main_thread){ //if it does not exist but current thread is main thread call yeild 
			fifo_yield();
		}
		return FAILURE; // if it does not exist and current thread is not main thread 
	}

	if (library->running == library->main_thread){ //if it exists but current thread is main thread call yeild 
		return fifo_yield();
	}

	// if it exits and current thread is not main put current thread behind it and call yeild
	
	fifo *current = library->running;
	current->waiting_tid = tid;
	fifo_enqueue(&library->waiting, current);

	if (current->state == RUNNING) {
		library->running->state = READY;
	} else {
		return FAILURE;
	}

	log_status(current, "STOPPED"); 
	fifo *next = fifo_dequeue(&library->ready);

	if (next == NULL){
		return FAILURE;
	}

	next->state = RUNNING;	
	library->running = next;
	log_status(next, "SCHEDULED");
	
	swapcontext(&current->context, &next->context);

	return EXIT_SUCCESS;
}

int fifo_libterminate(void){
	if(!innit) return FAILURE; 
	
	if (library->running == NULL || library->running != library->main_thread || library->ready != NULL){
		return FAILURE;
	}

	//fifo_terminate(&library->main_thread);
	free(library->main_thread);
	library->main_thread=NULL;

	fifo_terminate(&library->zombie);
	library->zombie=NULL;

	free(library);
	library=NULL;

	innit = false;
	return EXIT_SUCCESS;
}


