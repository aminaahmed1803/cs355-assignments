/*
 * parser.c
 *
 * Description:
 * This program is an implementation of the header file containing the
 * components that make up the parser.
 * This includes the implementation of the functions:
 *      init_tokenizer()
 *      get_next_token()
 *      get_command()
 */
#include "parser.h"

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


