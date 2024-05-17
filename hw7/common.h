#ifndef COMMON_H
#define COMMON_H

#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


#define RUN "running"
#define SUSP "suspended"
#define CONT "continued"
#define FIN "done"
#define TER "terminated"

#define SUCCESS 0
#define FAIL -1

#define TOKEN_DELIMITERS "&%<>"
#define COMMAND_DELIMITERS "\n\0;|"
#define MAXLEN 100
#define COMMAND_BUFSIZE 1024

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
    struct termios termios;
    bool termios_set;
} JOB;


int total_jobs;
char * line;
char ***commands;
pid_t shell_pid;

JOB *head;
JOB *foreground_job;
sigset_t blocked;
struct termios main_termios;

bool append;
bool redirect_in;
bool redirect_out;


char *read_line();
char ***parse_command(char *line, int *num_commands);

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
void free_node(JOB* job);

void check_zombie();
int wait_for_pid(int pid);
void signal_handler(int signal);
int set_status(int pid, int status);

void exiting();
int bg(char **command);
int fg(char **command);
int kill_job(char** command);


#endif