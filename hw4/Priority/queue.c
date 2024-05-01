#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// RR

void rr_enqueue(RR_TCB **head,RR_TCB *node){
   
   if (*head == NULL) {
      node->next = NULL;
      *head = node;
      return;
   }

  RR_TCB *tmp = *head;
   while (tmp->next != NULL) {
      tmp = tmp->next;
   }

   tmp->next = node;
   node->next = NULL;
}

RR_TCB * rr_dequeue(RR_TCB **head){
   if (*head == NULL) {
      return NULL;
   }

  RR_TCB *tmp = *head;
  *head = (*head)->next;
  tmp->next = NULL;
   return tmp;
}

void rr_queue_display(RR_TCB *head){
  RR_TCB *tmp = head;

    while (tmp != NULL) {
        printf("[%d]\t%d\t%d\t%d\n", 0, tmp->state, tmp->tid, tmp->priority);
        tmp = tmp->next;
    }
}

RR_TCB * rr_queue_search(RR_TCB **head, pid_t tid ){

   RR_TCB *tmp = *head;
   RR_TCB *prev = NULL;
    // If the head node itself holds the key to be deleted
   if (tmp != NULL && tmp->tid == tid) {
        *head = tmp->next;
        return tmp;
   }

    // Search for the key to be deleted, keep track of the previous node as well
   while (tmp != NULL && tmp->tid != tid) {
      prev = tmp;
      tmp = tmp->next;
   }

    // If the key was not present in the linked list
   if (tmp == NULL)
      return NULL;

    // Unlink the node from the linked list
   prev->next = tmp->next;
   return tmp;
}

void rr_queue_terminate(RR_TCB **head){
   
   if (*head == NULL) {
      return;
   }

  RR_TCB *current = *head;
  RR_TCB *tmp;
   while (current != NULL) {
      tmp = current;
      current = current->next;
      free(tmp->context.uc_stack.ss_sp);
      free(tmp);
   }

   *head = NULL;
}


RR_TCB * rr_pick_priority(RR_TCB *neg_one,RR_TCB *zero,RR_TCB* one){
	int weights[3];
   int total_sum = 0;

   if (neg_one == NULL) total_sum += 0; 
   else total_sum += NEG_WEIGHT;
   weights[0] = total_sum;

   if (zero == NULL) total_sum += 0; 
   else total_sum += ZERO_WEIGHT;
   weights[1] = total_sum;

   if (one == NULL) total_sum += 0; 
   else total_sum += ONE_WEIGHT;
   weights[2] = total_sum;

   if (total_sum == 0 ) return NULL;

   float randNum = (float) rand() / RAND_MAX;
   float target =  randNum * (total_sum);


   if (target < weights[0]){
      return rr_dequeue(&neg_one);
   }
   else if (target < weights[1]){
    
      return rr_dequeue(&zero);
   }
   else if (target < weights[2]){
      return rr_dequeue(&one);
   }
   return NULL;
}

unsigned long rr_get_elapsed_time(RR_TCB *node){
   struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the time difference in milliseconds
   unsigned long elapsed_ms = (current_time.tv_sec - node->start_time.tv_sec) * MILLISEC;
   elapsed_ms += (current_time.tv_usec - node->start_time.tv_usec) / MILLISEC;
   return elapsed_ms;
}


int rr_priority_enque(RR_TCB *node, RR_TCB **ready_one, RR_TCB **ready_zero, RR_TCB **ready_neg_one){

	if (node == NULL) {
		return FAILURE;
	}
	switch (node->priority){
		case 1:
			rr_enqueue(ready_one, node);	
			break;
		case 0:
			rr_enqueue(ready_zero, node);
			break;
		case -1:
			rr_enqueue(ready_neg_one, node);
			break;
		default:
			//free(*node->context.uc_stack.ss_sp);
			//free(*node);
			return FAILURE;
	}
	return EXIT_SUCCESS;
}