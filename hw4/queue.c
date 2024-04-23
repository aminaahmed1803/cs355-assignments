#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void  fifo_enqueue(FIFO_TCB **head, FIFO_TCB *node){
   
   if (*head == NULL) {
      node->next = NULL;
      *head = node;
      return;
   }

  FIFO_TCB *tmp = *head;
   while (tmp->next != NULL) {
      tmp = tmp->next;
   }

   tmp->next = node;
   node->next = NULL;
}

void fifo_enqueue_at_head(FIFO_TCB **head, FIFO_TCB *node){
  FIFO_TCB *tmp = *head;
   *head = node;
   node->next = tmp;
}

FIFO_TCB * fifo_dequeue(FIFO_TCB **head){
   if (*head == NULL) {
      return NULL;
   }

  FIFO_TCB *tmp = *head;
   *head = (*head)->next;
   return tmp;
}

void fifo_queue_display(FIFO_TCB *head){
  FIFO_TCB *tmp = head;

   while (tmp != NULL) {
      printf("[%d]\t%d\t%d\n", 0, tmp->state, tmp->tid);
      tmp = tmp->next;
   }
}

FIFO_TCB * fifo_queue_search(FIFO_TCB **head, pid_t tid ){
   if (head == NULL) {
      return NULL;
   }

  FIFO_TCB *tmp = *head;
  FIFO_TCB *prev = NULL;
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

void fifo_queue_terminate(FIFO_TCB **head){
   
   if (*head == NULL) {
      return;
   }

  FIFO_TCB *current = *head;
  FIFO_TCB *tmp;
   while (current != NULL) {
      tmp = current;
      current = current->next;

      free(tmp->context.uc_stack.ss_sp);
      free(tmp);
   }

   *head = NULL;
}


/// SJF

void sjf_enqueue(SJF_TCB **head, SJF_TCB *node){ // in acending order of t_n
   
   SJF_TCB* tmp;
   if (*head == NULL || (*head)->t_n > node->t_n) {
        node->next = *head;
        *head = node;
   }
   else {
        /* Locate the node before the point of insertion */
        tmp = *head;
        while (tmp->next != NULL && tmp->next->t_n < node->t_n) {
            tmp = tmp->next;
        }
        node->next = tmp->next;
        tmp->next = node;
   }
}

SJF_TCB * sjf_dequeue(SJF_TCB **head){
   if (*head == NULL) {
      return NULL;
   }

   SJF_TCB *tmp = *head;
   *head = (*head)->next;
   return tmp;
}

void sjf_queue_display(SJF_TCB *head){
   SJF_TCB *tmp = head;

   while (tmp != NULL) {
      printf("[%d]\t%d\t%d\n", 0, tmp->state, tmp->tid);
      tmp = tmp->next;
   }
}

SJF_TCB *  sjf_queue_search(SJF_TCB **head, pid_t tid ){
   if (head == NULL) {
      return NULL;
   }

   SJF_TCB *tmp = *head;
   SJF_TCB *prev = NULL;
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

void  sjf_queue_terminate(SJF_TCB **head){
   
   if (*head == NULL) {
      return;
   }

   SJF_TCB *current = *head;
   SJF_TCB *tmp;
   while (current != NULL) {
      tmp = current;
      current = current->next;
      free(tmp->context.uc_stack.ss_sp);
      free(tmp);
   }

   *head = NULL;
}

unsigned long  sjf_get_elapsed_time(SJF_TCB *node){
   struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the time difference in milliseconds
   unsigned long elapsed_ms = (current_time.tv_sec - node->start_time.tv_sec) * MILLISEC;
   elapsed_ms += (current_time.tv_usec - node->start_time.tv_usec) / MILLISEC;
   return elapsed_ms;
}
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