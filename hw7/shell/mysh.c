/*
 * mysh.c
 *
 * Description:
 * This program starts the myshell.
 */

#include "common.h"

int num_jobs = 1;
bool append;
bool redirect_in;
bool redirect_out;
char *filename;

int set_status(int pid, int status){
    if ((foreground_job != NULL) && (foreground_job->pid == pid)) {
        foreground_job->status = status;
        return SUCCESS;
    }

    JOB *job = remove_job_by_pid(&head, pid);

    if (job != NULL) {
        signal(SIGTTOU, SIG_IGN);
        job->status = status;
        signal(SIGTTOU, SIG_IGN);
    }

    return FAIL;
}

void signal_handler(int signal) {
    check_zombie();
}

void init() {
    total_jobs = 0;
    head = NULL;
    foreground_job = NULL;
    line = NULL;
    commands = NULL;

    /*struct sigaction sig_action = {
        .sa_handler = &signal_handler,
        .sa_flags = 0};
    sigemptyset(&sig_action.sa_mask);
    sigaction(SIGINT, &sig_action, NULL);*/
    signal(SIGCHLD, signal_handler);

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

int wait_for_pid(int pid) {
    int status = 0;

    waitpid(pid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        set_status(pid, FINISHED);
    } else if (WIFSIGNALED(status)) {
        set_status(pid, TERMINATED);
    } else if (WSTOPSIG(status)) {
        status = -1;
        set_status(pid, SUSPENDED);
    }
    
    return status;
}

void check_zombie(){
    int status, pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0){
        JOB *job = get_job(head, pid);
        if (job == NULL) {
            continue; 
        }


        if ( WIFSIGNALED(status)){
            job = remove_job_by_pid(&head, pid);
            job->status = TERMINATED;
            display_job_node(job);
            free_node(job);
            job = NULL;
            
        } else if (WIFEXITED(status)) { 
            job = remove_job_by_pid(&head, pid);
            job->status = FINISHED; //(pid, FINISHED)
            display_job_node(job);
            free_node(job);
            job = NULL;

        }

    }
}

int builtin_cmd_handler(char** command) {
    if (command == NULL || command[0] == NULL) {
        return FAIL;
    }

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

    if (strcmp(command[0], "ls") == 0) {
        //TODO
        return SUCCESS;
    }
    /*chmod changes the permissions mode of a file. 
    Support absolute mode and symbolic mode.*/
    if (strcmp(command[0], "chmod") == 0) {
        //TODO
        return SUCCESS;
    }
    /*mkdir creates a directory*/
    if (strcmp(command[0], "mkdir") == 0) {
        //TODO
        return SUCCESS;
    }
    /*rmdir removes a directory*/
    if (strcmp(command[0], "rmdir") == 0) {
        //TODO
        return SUCCESS;
    }
    /*cd changes the current working directory 
    according to the specified path, or to the 
    home directory if no argument is given. 
    Support . and ..  */
    if (strcmp(command[0], "cd") == 0) {
        //TODO
        return SUCCESS;
    }
    //pwd prints the current working directory
    if (strcmp(command[0], "pwd") == 0) {
        //TODO
        return SUCCESS;
    }
    //cat displays the content of one or more files to the output.
    //if (strcmp(command[0], "cat") == 0) {
        //TODO
    //    return SUCCESS;
    //}
    //more lists a file a screen at a time
    if (strcmp(command[0], "more") == 0) {
        //TODO
        return SUCCESS;
    }
    //rm deletes a file
    if (strcmp(command[0], "rm") == 0) {
        //TODO
        return SUCCESS;
    }
    /*mount mounts a file system at a specified 
    location.  Note that it should be possible to 
    format multiple disks and mount them at 
    arbitrary locations in your file system */
    if (strcmp(command[0], "mount") == 0) {
        //TODO
        return SUCCESS;
    }
    // umount unmounts a file system
    if (strcmp(command[0], "umount") == 0) {
        //TODO
        return SUCCESS;
    }

    return FAIL;
}

int check_redirection(char** command, int* in_fd, int* out_fd) {
    if (command == NULL) {
        return FAIL;
    }
    for (int i=0; command[i]!=NULL; i++){
        if (strcmp(command[i], ">") == 0) {
            if (command[i+1] != NULL && strcmp(command[i+1], ">") == 0) {
                if (command[i+2] == NULL){
                    return FAIL;
                }else {
                    filename = command[i+2];
                    //printf("append\n");
                    append = true;
                    *out_fd = open(filename, O_WRONLY | O_APPEND, 0644);
                    if (*out_fd <0){
                        *out_fd = 1;
                    }
                    free(command[i]);
                    command[i] = NULL;
                    free(command[i+1]);
                    command[i+1] = NULL;
                    command[i+2] = NULL;

                    return SUCCESS;
                }
            }else {
                if (command[i+1] == NULL){
                    return FAIL;
                }else {
                    //printf("redir out detected\n");
                    redirect_out = true;
                    filename = command[i+1];
                    *out_fd = open(filename, O_WRONLY | O_CREAT, 0644);
                    if (*out_fd <0){
                        *out_fd = 1;
                    }
                    free(command[i]);
                    command[i] = NULL;
                    command[i+1] = NULL;

                    return SUCCESS;
                }
            }
        } else if (strcmp(command[i], "<") == 0) {
            if (command[i+1] == NULL){
                return FAIL;
            }else{
                //printf("redir in detected\n");
                filename = command[i+1];

                redirect_in = true;
                *in_fd = open(command[i+1], O_RDONLY);
                if (*in_fd < 0){
                    printf("mysh: no such file or directory: %s\n", command[i+1]);
                    return FAIL;
                }

                free(command[i]);
                command[i] = NULL;
                command[i+1] = NULL;

                return SUCCESS;
            }
        }
    }
    return SUCCESS;
}

int launch_job(char*** command, int num_commands) {
    bool invalid = false;
    char invalid_command[1024] = "";
    int in_fd=0, out_fd=1;
    filename = NULL;

    if (num_commands == 0) {
        return SUCCESS;
    }

    for (int i = 0; i < num_commands; i++) {

        redirect_in = false;
        redirect_out = false;
        append = false;
        invalid = false;

        if (command[i] == NULL) {
            invalid = true;
            break;
        }        
    
        if ( builtin_cmd_handler(command[i]) == SUCCESS ) {
            continue;
        } 

        //check for redirection out, rediection in, append
        if (check_redirection(command[i], &in_fd, &out_fd) == FAIL) {
            invalid = true;
            break;
        }

        
        int sz = 0;
        JOB *job = (JOB *)malloc(sizeof(JOB));
        job->in_background = false;
        job->job_id = num_jobs;
        num_jobs++;
        job->pid = -1;
        job->status = RUNNING;

        for (; command[i][sz]!=NULL; sz++) {   
            if (strcmp(command[i][sz], "&") == 0) {
                if (command[i][sz+1]==NULL) {
                    job->in_background = true;
                    free(command[i][sz]);
                    command[i][sz] = NULL;
                } else{
                    return FAIL;
                } 
            }
        }

        sz += 2;
        job->command = (char **)malloc(sz *sizeof(char *));
        job->len = sz; 

        for (int k=0; k<sz; k++) {
            job->command[k] = NULL;
        }

        for (int k = 0; command[i][k] != NULL; k++) {
            job->command[k] = strdup(command[i][k]);

            if (job->command[k] == NULL) {
                perror("Failed to allocate memory for command argument");
                
                for (int j = 0; j < k; j++) {
                    free(job->command[j]);
                }
                free(job->command);
                free(job);
                return -1;
            }
        }
        
        pid_t childpid = fork();
        if (childpid < 0) {
            for (int k = 0; job->command[k] != NULL; k++) {
                free(job->command[k]);
            }

            free(job->command);
            free(job);
            return FAIL;
        } else if (childpid == 0) {
            job->pid =  getpid();
            setpgid(0, job->pid );

            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            
            
            if (redirect_in) {
                dup2(in_fd, 0);
                close(in_fd);
            }

            if (append) {
                dup2(out_fd, 1);
                close(out_fd);
            }

            if (redirect_out){
                dup2(out_fd, 1);
                close(out_fd);
            }

            if (execvp(job->command[0], job->command) < 0) {
                printf("%s: command not found\n", job->command[0]);

                for (int k = 0; job->command[k] != NULL; k++) {
                    free(job->command[k]);
                }
                free(job->command);
                free(job);
                exit(EXIT_FAILURE);
            }

            exit(0);
        } else {
            if (job->pid <= 0) {
                job->pid = childpid;
            }

            setpgid(0, childpid);
            int status = 0;

            if (job->in_background) {
                waitpid(childpid ,&status,WNOHANG|WUNTRACED);
                printf("[%d] %d\n", job->job_id, job->pid);
                
                insert_at_end(&head, job);
                job->job_id = ++total_jobs;
            } else {
                foreground_job = job;

                tcsetpgrp(0, childpid);
                int status = wait_for_pid(childpid);
                
                signal(SIGTTOU, SIG_IGN);
                tcsetpgrp(0, getpid());
                signal(SIGTTOU, SIG_DFL);

                if (job->pid <= 0) {
                    job->pid = childpid;
                }
        
                if (job->status == RUNNING) {
                    printf("error\n");
                    for (int k = 0; job->command[k] != NULL; k++) {
                        free(job->command[k]);
                    }

                    free(job->command);
                    free(job);
                    exit(0);
                }
    
                if (job->status == FINISHED || job->status == TERMINATED ) {
                    foreground_job = NULL;
                    free_node(job);
                    job = NULL;
                } else if (job->status == SUSPENDED) {
                    printf("zsh: suspended  ");

                    for (int i = 0; job->command[i] != NULL; i++) {
                        printf("%s ",job->command[i]);
                    }
                    printf("\n");

                    job->job_id = ++total_jobs;
                    insert_at_end(&head, job);
                }
            }

          
        } 
        if (filename != NULL) {
            free(filename);
            filename = NULL;
        }
        if (invalid) {
            printf("bash: syntax error near unexpected token '%s'\n", line);
        }
    
    }

    if (invalid) {
        printf("bash: syntax error near unexpected token '%s'\n", line);
    }
    

    return SUCCESS;
}

int main(int argc, char **argv) {
    init();
    
    bool run = true;
    while (run) {
        printf("[myshell] $ ");
        line = read_line(); 
        int num_commands = 0;
        
        if (line == NULL){
            printf("\n");
            exiting();
        }

        if (strlen(line) < 2){
            check_zombie();

            if (line != NULL) {
                free(line);
                line = NULL;
            }

            continue;
        }

        commands = parse_command(line, &num_commands);

        if (commands == NULL) {
            free(line); 
            line = NULL;
            continue;
        }

        if (launch_job(commands, num_commands) == FAIL){
            run = false;
        }

        free(line);
        line = NULL;
    
        if (commands != NULL) {
            for (int i = 0; commands[i] != NULL ; i++){
                for (int j = 0; commands[i][j] != NULL; j++){
                    free(commands[i][j]);
                    commands[i][j] = NULL;
                }
            
                free(commands[i]);
                commands[i] = NULL;
            }

            free(commands);
            commands = NULL;
        }

        free(line);
        line = NULL;
    }

    free(line);
    line = NULL;

    free_list(&head);
    head = NULL;
    foreground_job = NULL;
    
    return EXIT_SUCCESS;
}