#include <stdio.h>
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

#include"linked_jobs.h"
#include"parser.h"

#define SUCCESS 0
#define FAIL -1

int total_jobs; 
JOB *head;
JOB *foreground_job;
sigset_t blocked;

void check_zombie(int pid, int status);
int set_status(int pid, int status);
void exiting();
void sig_handler(int sig);

void sigchild_handler(int signal, siginfo_t *info, void *ucontext){
    
    pid_t pid = info->si_pid;

    if(info->si_code == CLD_EXITED){
        check_zombie(pid, FINISHED);
    }
    if(info->si_code == CLD_KILLED || info->si_code == CLD_DUMPED){
        check_zombie(pid, TERMINATED);
    }
    else if (info->si_code == CLD_STOPPED) { // child has stopped
        check_zombie(pid, SUSPENDED);
    }
    else if (info->si_code == CLD_CONTINUED) { // Stopped child has continued
        set_status(pid, CONTINUED);
    }

    return;
}

void init(){

    total_jobs = 0;
    head = NULL;

    struct sigaction sig_action = {
        .sa_flags = SA_SIGINFO,
        .sa_sigaction = &sigchild_handler};
    sigemptyset(&sig_action.sa_mask);
    sigaction(SIGCHLD,&sig_action,NULL);

    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    pid_t pid = getpid();
    setpgid(pid, pid);
    tcsetpgrp(0, pid);
}


int set_status(int pid, int status){

    if (foreground_job->pid == pid){
        foreground_job->status = status;
        return SUCCESS;
    }

    JOB *job = remove_job_by_pid(&head, pid);
    if (job != NULL){
        signal(SIGTTOU, SIG_IGN);
        job->pid = status;
        signal(SIGTTOU, SIG_IGN);
    }

    return FAIL;
}

void check_zombie(pid_t pid, int status){

    JOB *job = remove_job_by_pid(&head, pid);
    if (job == NULL){
        return; 
    }

    if (status == SUSPENDED){
        job->status = SUSPENDED;
        insert_at_end(&head, job);
        return;
    }
    
    signal(SIGTTOU, SIG_IGN);
    job->status = status;
    signal(SIGTTOU, SIG_IGN);
    printf("\n");
    display_job_node(job);
    free_node(&job);
}

// add remove job here 
int wait_for_pid(int pid){
    int status = 0;

    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)){
        set_status(pid, FINISHED);
    }
    else if (WIFSIGNALED(status)){
        set_status(pid, TERMINATED);
    }
    else if (WSTOPSIG(status)){
        status = -1;
        set_status(pid, SUSPENDED);
    }
    
    return status;
}

void exiting() {
    
    free_list(&head);
    kill(0, SIGTERM);
    printf("logout\n");
    printf("Connection to myshell closed.\n");
    exit(0);
}

int kill_job(char** command) {
    
    int start = 1;
    pid_t pid;
    int job_id = -1;
    JOB *job = NULL; 

    if (command[start] == NULL) {
        printf("kill: usage: kill [-s sigspec | -n signum | -sigspec] pid | jobspec ... or kill -l [sigspec]\n");
        return FAIL;
    }

    bool is_sigkill = false;
    if (strcmp(command[start], "-9") == 0) {
        is_sigkill = true;
        start++;
    }

    for (int i = start; command[i] != NULL; i++) {
        
        pid_t pid = atoi(command[i]);
        job = get_job(head, pid);

        if (job == NULL) {
            printf("-bash: kill: (%s) - No such process\n", command[i]);
            continue;
        }
            // Determines which signal to send SIGKILL or SIGTERM
            // If is_sigkill is true, sends SIGKILL
            // If is_sigkill is false, sends SIGTERM
        pid = job->pid;
        int signal;
        if (is_sigkill){
            signal = kill(pid, SIGKILL);
        }else { 
            signal = kill(pid, SIGTERM);
        }
        if (signal < 0) {
               // Error occurred
            printf("Failed to send signal to job (%s)\n", command[i]);
        } else {
               // Success, signal sent
            printf("Sent signal to job (%s)\n", command[i]);
        }
        //wait_for_pid(pid);
    }  
    return SUCCESS;
}

/*bring backgrounded job #
 back into the foreground*/
int fg(char **command){

    if (command == NULL){
        return FAIL;
    }
    if (head == NULL){
        printf("bash: fg: current: no such job\n");
        return SUCCESS;
    }

    JOB* job;
    pid_t pid;
    int job_id = -1;

    if (command[1] == NULL){
        job = get_last_SUSPENDED(head);
        if (job == NULL){
            printf("bash: fg: current: no such job\n");
            return SUCCESS;
        }

        pid = job->pid;
        job_id = job->job_id;
        remove_job_by_pid(&head, pid);
        
    } else if (command[1][0] == '%'){
        if (command[2] == NULL){
            printf("mysh: fg %s: no such job\n", command[2]);
            return FAIL;
        }
        job_id = atoi(command[2]);
        JOB* job = remove_job_by_id(&head, job_id);
        if (job == NULL){
            printf("mysh: fg %s: no such job\n", command[2]);
            return FAIL;
        }else {
            pid = job->pid;
        }
    } else {
        pid = atoi(command[1]);
         
        job = remove_job_by_pid(&head, pid);
       
        if (job == NULL){
            printf("mysh: fg %s: no such job\n", command[1]);
            return FAIL;
        }
        job_id = job->job_id;
        pid = job->pid;
    }

    foreground_job = job;
    job->status = CONTINUED;
    job->in_background=false;

    if (kill(-pid, SIGCONT) < 0){
        printf("mysh: fg %d: job not found\n", pid);
        return -1;
    }
    
    if (tcsetpgrp(0, pid) < 0){
        printf("can't bring job t fg");
    }
    wait_for_pid(pid);

    signal(SIGTTOU, SIG_IGN);
    tcsetpgrp(0, getpid());
    signal(SIGTTOU, SIG_DFL);


    if(job->status == RUNNING){
        printf("error\n");
        exit(0);
    }

    foreground_job = NULL;
    
    if (job->status == FINISHED || job->status == TERMINATED ){
        free_node(&job);
    } else if (job->status == SUSPENDED){
            //print out statment
        printf("zsh: suspended  ");
        for (int i=0; job->command[i]!=NULL; i++){
            printf("%s ",job->command[i]);
        }
        printf("\n");
        insert_at_end(&head, job);
    }
    
    return SUCCESS;
}

/*If job # is omitted, 
shell should background 
the last job suspended, if any.*/
int bg(char **command){
    if (command == NULL){
        return FAIL;
    }
    if (head == NULL){
        printf("bash: bg: current: no such job\n");
        return SUCCESS;
    }

    JOB* job;
    pid_t pid;
    int job_id = -1;

    if (command[1] == NULL){
        job = get_last_SUSPENDED(head);
        if (job == NULL){
            printf("bash: bg: current: no such job\n");
            return SUCCESS;
        }
        pid = job->pid;
        job_id = job->job_id;
        
    } else if (command[1][0] == '%'){
        if (command[2] == NULL){
            printf("mysh: bg %s: no such job\n", command[2]);
            return FAIL;
        }
        job_id = atoi(command[2]);
        JOB* job = get_job_by_id(head, job_id);
        if (job == NULL){
            printf("mysh: bg %s: no such job\n", command[1]);
            return FAIL;
        }else {
            pid = job->pid;
        }
    }else {
        pid = atoi(command[1]);
        job = get_job(head, pid);
        if (job == NULL){
            printf("mysh: bg %s: no such job\n", command[1]);
            return FAIL;
        }
        job_id = job->job_id;
    } 

    if (kill(-pid, SIGCONT) < 0){
        printf("mysh: bg %d: job not found\n", pid);
        return -1;
    }
    
    job->in_background=true;
    if (job_id > 0){
        set_status(pid, CONTINUED);
        display_job_node(job);
    }
    
    return 0;
}

int launch_background(JOB *job){
    
    pid_t childpid = fork();
    if (childpid < 0){
        return FAIL;
    }
    else if (childpid == 0){
        job->pid =  getpid();
        setpgid(0, job->pid );

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        if (execvp(job->command[0], job->command) < 0){
            printf("%s: command not found\n", job->command[0]);
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    } else {

        if (job->pid <= 0){
            job->pid = childpid;
        }
        printf("[%d] %d\n", job->job_id, job->pid);
        setpgid(0, childpid);
    }
    return SUCCESS;
}

int launch_foreground(JOB *job){

    pid_t childpid = fork();
    int job_pid;
    if (childpid < 0){
        return FAIL;
    }
    
    else if (childpid == 0){

        job->pid =  getpid();
        setpgid(0, job->pid ); 

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        if (execvp(job->command[0], job->command) < 0){
            printf("%s: command not found\n", job->command[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);

    } else {    

        if (job->pid <= 0){
            job->pid = childpid;
        }
        
        setpgid(0, childpid);
        tcsetpgrp(0, childpid);
        wait_for_pid(childpid);
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(0, getpid());
        signal(SIGTTOU, SIG_DFL);

        if(job->status == RUNNING){
            printf("error\n");
            exit(0);
        }
    
        if (job->status == FINISHED || job->status == TERMINATED ){
            foreground_job = NULL;
            free_node(&job);
        }
        else if (job->status == SUSPENDED){
            //print out statment
            printf("zsh: suspended  ");
            for (int i=0; job->command[i]!=NULL; i++){
                printf("%s ",job->command[i]);
            }
            printf("\n");

            //add job to back grounded jobs. 
            job->job_id = ++total_jobs;
            insert_at_end(&head, job);
        }
    }
    return SUCCESS;
}

int builtin_cmd_handler(char** command){
    
    if (strcmp(command[0], "exit") == 0) { 
        exiting();
    } 
    if (strcmp(command[0], "jobs") == 0) { 
        display_jobs_list(head);
        return SUCCESS;
    }
    if (strcmp(command[0], "kill") == 0) { 
        kill_job(command);
        return SUCCESS;
    } 
    if (strcmp(command[0], "fg") == 0) {
        fg(command);
        return SUCCESS;
    } 
    if (strcmp(command[0], "bg") == 0) {
        bg(command);
        return SUCCESS;
    }
    return FAIL;
}

int launch_job(char*** command, int num_commands){

   //acknowledge_zombie();
    if (num_commands == 0){
        return SUCCESS;
    }

    for(int i=0; i<num_commands; i++){

        if (command[i] == NULL){
            printf("command not found\n");
            continue;
        }
        
        if (builtin_cmd_handler(command[i]) == SUCCESS){
            continue;
        } 
        
        int sz = 0;
        JOB *new_job = (JOB *)malloc(sizeof(JOB));
        new_job->in_background = false;
        new_job->job_id = -1;
        new_job->pid = -1;
        new_job->status = RUNNING;

        for (; command[i][sz]!=NULL; sz++){   
            if (strcmp(command[i][sz], "&") == 0){
                if (command[i][sz+1]==NULL){
                    new_job->in_background = true;
                    free(command[i][sz]);
                    command[i][sz] = NULL;
                }else{
                    return FAIL;
                } 
            }
        }
        sz += 2;
        new_job->command = (char **)malloc(sz *sizeof(char *));
        new_job->len = sz; 
        for (int k=0; command[i][k]!=NULL; k++){
            new_job->command[k] = strdup(command[i][k]);
        }
        
        if (new_job->in_background){
            insert_at_end(&head, new_job);
            new_job->job_id = ++total_jobs;
            if (launch_background(new_job) == FAIL)
                return FAIL; 
        }
        else {
            foreground_job = new_job;
            if (launch_foreground(new_job) == FAIL)
                return FAIL; 
        }
        
    }

    return SUCCESS;
}
    
int main(int argc, char **argv){
    init();
    
    bool run = true;
    while (run){
        printf("[myshell] $ ");
        char *line = read_line(); 
        int num_commands = 0;
        
        if (line == NULL){
            continue;
        }
        if (strlen(line) < 2 ){
            free(line);
            continue;
        }
        
        char ***commands = parse_command(line, &num_commands);
        free(line);

        if (launch_job(commands, num_commands) == FAIL){
            run = false;
        }
    
        if(commands!= NULL){
            for (int i=0; commands[i] != NULL ; i++ ){
                for (int j=0;commands[i][j] != NULL; j++ ){
                    free(commands[i][j]);
                }
            free(commands[i]);
            }
        }

        free(commands);
    }
    return EXIT_SUCCESS;
}