#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <stdbool.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


#define TOKEN_DELIMITERS "&%<>"
#define COMMAND_DELIMITERS "\n\0;|"
#define MAXLEN 100
#define COMMAND_BUFSIZE 1024
#define PATH_BUFSIZE 1024
#define COMMAND_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define FAIL -1
#define SUCCESS 0



struct shell_info
{
    char cur_user[TOKEN_BUFSIZE];
    char cur_dir[PATH_BUFSIZE];
    char pw_dir[PATH_BUFSIZE];
};

struct shell_info *shell;


void mysh_update_cwd_info()
{
    getcwd(shell->cur_dir, sizeof(shell->cur_dir));
}

int mysh_cd(int argc, char **argv)
{
    if (argc == 1)
    {
        chdir(shell->pw_dir);
        mysh_update_cwd_info();
        return 0;
    }

    if (chdir(argv[1]) == 0)
    {
        mysh_update_cwd_info();
        return 0;
    }
    else
    {
        printf("mysh: cd %s: No such file or directory\n", argv[1]);
        return 0;
    }
}

int mysh_export(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: export KEY=VALUE\n");
        return -1;
    }

    return putenv(argv[1]);
}

int mysh_unset(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: unset KEY\n");
        return -1;
    }

    return unsetenv(argv[1]);
}

int mysh_exit()
{
    printf("Goodbye!\n");
    exit(0);
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
        
        
    }

    return SUCCESS;
}


char *trim_string(char *line){
    if (line == NULL){
        return NULL;
    }

    bool null_terminate = false; 
    char *head = line, *tail = line + strlen(line) -1;
    while (isspace(*head)){
        head++;
    }
    while (isspace(*tail)){
        tail--;
        null_terminate = true;
    }
    if (null_terminate){
        *(tail + 1) = '\0';
    }
    return head;
}

char **tokenize_commands(char *segment){

    if (segment == NULL || strlen(segment) < 2){
        return NULL;
    }

    segment = trim_string(segment);
    char *token_cursor = segment, *c = segment, *token;
    int token_len = 0, idx = 0;
    char **all_tokens = (char**) malloc(strlen(segment) * sizeof(char*));

    for(int i=0; i<strlen(segment); i++){
        all_tokens[i] = NULL;
    }

    while (true){
        if (*c == '\0' || *c == '&' || *c == '%' || *c == '<' || *c == '>'|| isspace(*c) ){
            
            if (token_len != 0){
                token = (char *)malloc((token_len + 1) * sizeof(char));
                strncpy(token, token_cursor, token_len);
                token[token_len] = '\0';
                all_tokens[idx] = token;  
                idx+=1;  
            }    
            
            if (*c != '\0'){

                if (!isspace(*c)){
                // add the delimiter to the array of tokens
                    char *tmp = NULL;
                    int len = 1;
                    if (*c == '&'){
                        tmp = "&";
                    }
                    else if (*c == '%'){
                        tmp = "%";
                    }
                    else if (*c == '<'){
                        tmp = "<";
                    }
                    else if (*c == '>'){
                        tmp = ">";
                    }

                    char *delim = (char*)malloc((len+1) * sizeof(char));
                    strncpy(delim, tmp, len);
                    delim[len] = '\0';
                    // increment by that much
                    all_tokens[idx] = delim;
                    idx += 1;
                    c += len;
                     
                }
                token_cursor = c;
                while (isspace(*token_cursor)){
                    token_cursor+=1;
                }
                c = token_cursor;
                token_len = 0;
                continue;
            }
            else{
                break;
            }
        }
        else{
            token_len++;
            c++;
        }
    }
    
    //all_tokens = (char**) realloc(all_tokens, (idx+1) * sizeof(char*));
    return all_tokens;
}

char ***parse_command(char *line, int *num_commands){
    if (line == NULL || strlen(line) < 2){
        return NULL;
    }

    line = trim_string(line);

    char *line_cursor = line, *c = line, *seg;
    int seg_len = 0, idx=0;

    char ***all_commands = (char***) malloc(strlen(line) * sizeof(char**));
    if (all_commands == NULL){
        exit(0);
    }

    for(int i=0; i<strlen(line); i++){
        all_commands[i] = NULL;
    }
    
    while (true){
        if (*c == '\0' || *c == '\n' || *c == ';' || *c == '|'){
            *num_commands += 1;
            seg = (char *)malloc((seg_len + 1) * sizeof(char));
            strncpy(seg, line_cursor, seg_len);
            seg[seg_len] = '\0';

            //call the other one to get segmented command
            all_commands[idx] = tokenize_commands(seg);
            idx +=1;
            free(seg);

            if (*c != '\0'){
                line_cursor = c;
                while (isspace(*(++line_cursor)));
                c = line_cursor;
                seg_len = 0;
                continue;
            }
            else{
                break;
            }
        }
        else{
            seg_len++;
            c++;
        }
    }
    
    //all_commands = (char***) realloc(all_commands, (idx+1) * sizeof(char**));
    return all_commands;
}

char *read_line(){
    
    int bufsize = COMMAND_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer){
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1){
        c = getchar();

        /*if (c == EOF){
            
            strncpy(buffer, "exit", 5);
            buffer[4] = '\0';
            return buffer;
        } */

        if (c == '\n' || c == EOF){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize){
            bufsize += COMMAND_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}



void mysh_init()
{
    

    shell = (struct shell_info *)malloc(sizeof(struct shell_info));
    getlogin_r(shell->cur_user, sizeof(shell->cur_user));

    struct passwd *pw = getpwuid(getuid());
    strcpy(shell->pw_dir, pw->pw_dir);

    mysh_update_cwd_info();
}

int main(int argc, char **argv)
{
    mysh_init();



    return EXIT_SUCCESS;
}
