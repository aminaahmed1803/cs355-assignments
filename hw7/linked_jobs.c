/*
 * linked_jobs.c
 *
 * Description:
 * This program is an implementation of the header file containing the
 * components that make up a doubly linked list for a list of jobs.
 * This includes the implementation of the functions:
 *      display_jobs_list()
 *      display_job_node()
 *      insert_at_end()
 *      remove_job_by_pid()
 *      remove_job_by_id()
 *      remove_last()
 *      get_job()
 *      get_job_by_id()
 *      free_list()
 *      free_node()
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
int global_job_id = 1;



/*
 * display_jobs_list()
 * -------------------
 * Prints out and displays each job’s details to the standard output to
 * list all jobs.
 *
 * parameters: head - a job node pointing to the head of the jobs list.
 * returns: returns nothing. 
 */
void display_jobs_list(JOB* head) {
    JOB* current = NULL; // Initialize current to NULL

    if (head != NULL) {
        current = head; // Assign head to current only if head is not NULL
        while (current != NULL) {
            display_job_node(current);
            current = current->next;
        }
    } else {
        printf("No jobs to print\n");
    }
}


/*
 * display_job_node()
 * ------------------
 * Prints out and displays the specified job’s details to the standard output.
 *
 * parameters: node - a job node.
 * returns: returns nothing. 
 */
void display_job_node(JOB* node) {
    if (node == NULL){
        return;
    }

    char* stat;
    if (node->status == 0) {
      stat = RUN;
    } else if (node->status == 1) {
      stat = SUSP;
    } else if (node->status == 2) {
      stat = CONT;
    } else if (node->status == 3) {
      stat = FIN;
    } else {
      stat = TER;
    }

    printf("[%d]  +  %s    ", node->job_id, stat);
    for (int i=0; node->command[i] != NULL; i++){
        printf("%s ", node->command[i]);
    }
    
    printf("\t%d\n", node->pid);
}

/*
 * insert_at_end()
 * ---------------
 * Inserts a new job node at the end of the linked list.
 *
 * parameters:
 * head - a pointer to a pointer to the head of the linked list.
 * new_node - a pointer to the new job node to be inserted.
 *
 * returns: returns nothing. 
 */
void insert_at_end(JOB** head, JOB* new_node) {
    if (*head == NULL) {
        *head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        return;
    }

    JOB* end = *head;

    while (end->next != NULL) {
        end = end->next;
    }

    end->next = new_node;
    new_node->prev = end;
    new_node->next = NULL;
}

/*
 * remove_job_by_pid()
 * ------------
 * Removes a job with the specified pid from the jobs list and reconnects the
 * job nodes in the list accordingly.
 *
 * parameters:
 * head - a job node pointing to a pointer of the head of the jobs list
 *        (a pointer to a pointer since we need to modify the value).
 * pid - an integer that represents the process ID of the
 *       job to be removed.
 *
 * returns: returns nothing. 
 */
JOB* remove_job_by_pid(JOB** head, pid_t pid_del) {
    if (*head == NULL) {
        //printf("In remove job: Head node cannot be NULL.\n");
        return NULL;
    }

    JOB* current = *head;

    while (current != NULL) {  // Loop through jobs
        if (current->pid == pid_del) {
            if (current == *head) {  // Checks if head
                *head = current->next;
            }

            // Checks if there is a next job node and readjusts accordingly
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }

            // Checks if there is a prev job node and readjusts accordingly
            if (current->prev != NULL) {
                current->prev->next = current->next;
            }

            return current;
        }

        current = current->next;
    }

    //printf("In remove job: Pid %d not found, unable to remove the job.\n", pid_del);
    return NULL;
}

/*
 * remove_last()
 * -----------------
 * Removes job by id.
 *
 * parameters:
 * head - a pointer to the head node.
 *
 * returns: returns a JOB.
 */
JOB* remove_job_by_id(JOB** head, int job_id){
    if (*head == NULL) {
        //printf("remove_job_by_id: Head node cannot be NULL.\n");
        return NULL;
    }

    JOB* current = *head;

    while (current != NULL) {  // Loop through jobs
        if (current->job_id == job_id) {
            if (current == *head) {  // Checks if head
                *head = current->next;
            }

            // Checks if there is a next job node and readjusts accordingly
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }

            // Checks if there is a prev job node and readjusts accordingly
            if (current->prev != NULL) {
                current->prev->next = current->next;
            }

            return current;
        }

        current = current->next;
    }

    //printf("remove_job_by_id: id %d not found, unable to remove the job.\n", job_id);
    return NULL;
}

/*
 * remove_last()
 * -----------------
 * Removes the last job from the jobs list and reconnects the
 * job nodes in the list accordingly.
 *
 * parameters:
 * head - a pointer to the head node.
 *
 * returns: returns a JOB.
 */
JOB* remove_last(JOB** head) {
    if (*head == NULL) {
        //printf("remove_last: Head node cannot be NULL.\n");
        return NULL;
    }

    JOB* current = *head;

    while (current != NULL) {
        if (current->next == NULL) {
            if (current->prev != NULL) {
                current->prev->next = NULL;
                return current;
            } else {
                *head = NULL;
                return current;
            }
        }

        current = current->next;
    }

    //printf("remove_last: Last job not found, unable to remove the job.\n");
    return NULL;
}

/*
 * get_job()
 * ---------
 * Given a pid, returns the job with the same pid.
 *
 * parameters:
 * head - a head JOB node.
 * pid - an integer pid.
 * 
 * returns: returns a job node.
 */
JOB* get_job(JOB* head, pid_t pid) {
    if (head == NULL) {
        //printf("get_job: Head node cannot be NULL.\n");
        return NULL;
    }

    JOB* current = head;

    while (current != NULL) {  // Loop through jobs
        // Return the job if job with same pid is found
        if (current->pid == pid) {
            return current;
        }

        current = current->next;
    }

    //printf("get_job: Pid %d not found, unable to get the job.\n", pid);
    return NULL;
}

/*
 * get_job_id()
 * ------------
 * Given a job_id, returns the job with the same job_id.
 *
 * parameters:
 * head - the head JOB node.
 * job_id - an integer job_id.
 * 
 * returns: returns a job node.
 */
JOB* get_job_by_id(JOB* head, int job_id) {
    if (head == NULL) {
        //printf("get_job_by_id: Head node cannot be NULL.\n");
        return NULL;
    }

    JOB* current = head;

    while (current != NULL) {  // Loop through jobs
        // Return the job if job with same pid is found
        if (current->job_id == job_id) {
            return current;
        }

        current = current->next;
    }

    //printf("get_job_by_id: Job ID %d not found, unable to get the job.\n", job_id);
    return NULL;
}

/*
 * free_list()
 * ----------------
 * Frees the jobs in the jobs list, in other words, deallocates
 * memory for all nodes in the jobs linked list.
 *
 * parameters:
 * head - a job node pointing to a pointer of the head of the jobs list
 *        (a pointer to a pointer since we need to modify the value).
 * 
 * returns: returns nothing.
 */
void free_list(JOB** head) {
    if (head == NULL) {
        //printf("free_list: Head node cannot be NULL.\n");
        return;
    }

    JOB* current = *head;
    JOB* temp = NULL;

    while ((current != NULL) && (current->next != NULL)) {
        temp = current;
        current = current->next;
        free(temp);
    }

    *head = NULL;
}

/*
 * get_last_SUSPENDED()
 * --------------------
 * Gets the last job with the status SUSPENDED.
 *
 * parameters:
 * head - a job node pointing to a pointer of the head of the jobs list
 *        (a pointer to a pointer since we need to modify the value).
 * 
 * returns: returns the job with status SUSPENDED.
 */
JOB* get_last_SUSPENDED(JOB* head) {
    if (head == NULL){
        //printf("get_last_SUSPENDED: no jobs.\n");
        return NULL;
    }
    JOB* current = head;
    JOB* last_SUSPENDED = NULL;

    while (current != NULL) {
        if (current->status == SUSPENDED) {
            last_SUSPENDED = current;
        }

        current = current->next;
    }

    // if (last_SUSPENDED == NULL) {
    //     printf("get_last_SUSPENDED: There is no SUSPENDED node.\n");
    // }

    return last_SUSPENDED;
}

/*
 * free_node()
 * -----------------
 * Frees the specified job node.
 *
 * parameters:
 * job - a JOB node.
 *
 * returns: returns nothing.
 */
void free_node(JOB* job) {
    if (job == NULL) {
        //printf("free_node: node cannot be NULL.\n");
        return;
    }

    JOB* node = job;

    if (node->command != NULL){
        for (int i = 0; node->command[i] != NULL; i++){
            free(node->command[i]);
            node->command[i] = NULL;
        }

        free(node->command);
        node->command = NULL;
    }

    free(node);
    node = NULL;
    job = NULL;
            
    return;
    
}