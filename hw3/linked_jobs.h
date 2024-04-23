/*
 * linked_jobs.h
 *
 * Description:
 * This program is a header file containing the components that make up
 * a doubly linked list for a list of jobs. This includes a job struct and
 * respective functions for a doubly linked list of jobs.
 */

#ifndef LINKED_JOBS_H
#define LINKED_JOBS_H
#include <sys/types.h> 
#include <stdbool.h>


#define RUN "running"
#define SUSP "suspended"
#define CONT "continued"
#define FIN "done"
#define TER "terminated"


typedef enum status {
    RUNNING,
    SUSPENDED,
    CONTINUED,
    FINISHED,
    TERMINATED
} STATUS;

// A structure for a [job] node in the doubly linked list.
typedef struct job{

    char **command;
    pid_t pid;
    int job_id;
    bool in_background;
    STATUS status;
    int len;
    struct job *next;
    struct job* prev;
    
    
} JOB;

void display_jobs_list(JOB* head);
void display_job_node(JOB* node);
void insert_at_end(JOB** head, JOB* new_node);
JOB* remove_job_by_pid(JOB** head, pid_t pid);
JOB* remove_job_by_id(JOB** head, pid_t pid);
JOB* remove_last(JOB** head);
JOB* get_job(JOB* head, pid_t pid);
JOB* get_job_by_id(JOB* head, int job_id);
void free_list(JOB** head);
JOB* get_last_SUSPENDED(JOB* head);
void free_node(JOB** job);

#endif
                