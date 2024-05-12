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

#include "common.h"

char *trim_string(char *line) {
    if (line == NULL){
        return NULL;
    }

    bool null_terminate = false; 
    char *head = line, *tail = line + strlen(line) -1;

    while (isspace(*head)) {
        head++;
    }

    while (isspace(*tail)) {
        tail--;
        null_terminate = true;
    }

    if (null_terminate) {
        *(tail + 1) = '\0';
    }

    return head;
}

void free_tokens(char **tokens) {
    if (tokens == NULL) {
        return;
    }

    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
        tokens[i] = NULL;
    }

    free(tokens);
    tokens = NULL;
}

char **tokenize_commands(char *segment) {
    if ((segment == NULL) || (strlen(segment) < 2)) {
        return NULL;
    }

    segment = trim_string(segment);
    char *token_cursor = segment, *c = segment, *token;
    int token_len = 0, idx = 0;
    char **all_tokens = (char**)malloc(strlen(segment) * sizeof(char*));

    if (all_tokens == NULL) {
        fprintf(stderr, "ERROR: Memory allocation error\n");
        return NULL;
    }

    for(int i = 0; i < strlen(segment); i++) {
        all_tokens[i] = NULL;
    }

    while (true) {
        if (*c == '\0' || *c == '&' || *c == '%' || *c == '<' || *c == '>'|| isspace(*c) ){
            
            if (token_len != 0){
                token = (char *)malloc((token_len + 1) * sizeof(char));

                if (token == NULL) {
                    fprintf(stderr, "ERROR: Memory allocation error\n");
                    return NULL;
                }

                for (int i=0; i<=token_len; i++) {
                    token[i] = '\0';
                }

                strncpy(token, token_cursor, token_len);
                token[token_len] = '\0';
                all_tokens[idx] = token;  
                idx+=1;  
            }    
            
            if (*c != '\0') {
                if (!isspace(*c)) {
                // add the delimiter to the array of tokens
                    char *tmp = NULL;
                    int len = 1;
                    if (*c == '&'){
                        tmp = "&";
                    } else if (*c == '%'){
                        tmp = "%";
                    } else if (*c == '<'){
                        tmp = "<";
                    } else if (*c == '>'){
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
                while (isspace(*token_cursor)) {
                    token_cursor+=1;
                }

                c = token_cursor;
                token_len = 0;
                continue;
            } else {
                break;
            }
        } else {
            token_len++;
            c++;
        }
    }

    return all_tokens;
}

char ***parse_command(char *line, int *num_commands) {
    if ((line == NULL) || (strlen(line) < 2)) {
        printf("bash: syntax error near unexpected token '%s'\n", line);
        return NULL;
    }

    /*if (line[0] != '\0' && line[1] != '\0') {
        if (line[0] == 'p' && line[1] == ' ') {
            printf("bash: syntax error near unexpected token p\n");
            return NULL;
        }
    }*/

    line = trim_string(line);

    char *line_cursor = line, *c = line, *seg;
    int seg_len = 0, idx=0;

    char ***all_commands = (char***)malloc(strlen(line) * sizeof(char**) + 1);

    if (all_commands == NULL) {
        fprintf(stderr, "ERROR: Memory allocation error\n");
        return NULL;
    }

    for (int i = 0; i < strlen(line); i++) {
        all_commands[i] = NULL;
    }
    
    while (true) {
        if (*c == '\0' || *c == '\n' || *c == ';' || *c == '|') {
            if (seg_len > 0) {
                *num_commands += 1;
                seg = (char *)malloc((seg_len + 1) * sizeof(char));
                strncpy(seg, line_cursor, seg_len);
                seg[seg_len] = '\0';

                //call the other one to get segmented command
                all_commands[idx] = tokenize_commands(seg);
                idx += 1;
                free(seg);
                seg = NULL;
            }

            if (*c != '\0') {
                line_cursor = c;
                while (isspace(*(++line_cursor)));
                c = line_cursor;
                seg_len = 0;
                continue;
            } else {
                break;
            }
        } else if (*c == '&') {
            if (seg_len > 0) {
                *num_commands += 1;
                seg = (char *)malloc((seg_len + 2) * sizeof(char));

                if (seg == NULL) {
                    fprintf(stderr, "ERROR: Memory allocation error\n");
                    return NULL;
                }

                strncpy(seg, line_cursor, seg_len);
                seg[seg_len] = *c;
                seg[seg_len + 1] = '\0';

                //call the other one to get segmented command
                all_commands[idx] = tokenize_commands(seg);
                idx += 1;
                free(seg);
                seg = NULL;
                seg_len = 0;
            }

            if (*c != '\0') {
                line_cursor = c + 1; // move the cursor to the next character
                c = line_cursor;
                continue;
            } else {
                break;
            }
        } else {
            seg_len++;
            c++;
        }
    }
    
    // Print the values of all_commands
    // printf("Parsed Commands:\n");
    // for (int i = 0; i < *num_commands; i++) {
    //     printf("%d. ", i + 1);
    //     char **command = all_commands[i];
    //     int j = 0;
    //     while (command[j] != NULL) {
    //         printf("%s ", command[j]);
    //         j++;
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    return all_commands;
}

char *read_line() {
    int bufsize = COMMAND_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        /*if (c == EOF){
            free(buffer);
            buffer = NULL;
            return NULL;
            //strncpy(buffer, "exit", 5);
            //buffer[4] = '\0';
            //return buffer;
        } else */if (c == '\n' || c == EOF ) {
            buffer[position] = '\0';
            break;
        } else {
            buffer[position] = c;
        }

        position++;

        if (position >= bufsize) {
            bufsize += COMMAND_BUFSIZE;
            char *new_buffer = realloc(buffer, sizeof(char) * bufsize);

            if (!new_buffer) {
                fprintf(stderr, "mysh: allocation error\n");
                free(buffer); // Free the original buffer before exit
                buffer = NULL;
                exit(EXIT_FAILURE);
            }

            buffer = new_buffer;
        }
    }

    return buffer;
}