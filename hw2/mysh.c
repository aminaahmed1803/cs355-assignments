#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>


#define HISTSIZE 50

/*
 * Function:  built_in_cmd_handler
 * --------------------
 */
int built_in_cmd_handler(char* cmd) { 
	
   if (strcmp(cmd, "exit") == 0){
      free(cmd);
      printf("\nConnection to shell closed.\n");
      exit(1);
   }
   return 1;
} 

/*
 * Function:  simple_cmd_handler
 * --------------------
 */
int simple_cmd_handler(char* command, char** args_list){
   pid_t pid = fork(); 

	if (pid == -1) { 
		printf("Failed forking child..\n"); 
		return -1; 
	}
   
   if (pid == 0) { 
      int ex = execvp(command, args_list) ;
		if (ex < 0) { 
			printf("Could not execute command..\n"); 
         return 1; 
		} 
		exit(0); 
	} else { 
		wait(NULL); 
		return 0; 
	} 
   return 0; 
}

void history_management(char* cmd, char** arguments){
   
   if (strcmp(cmd, "!!") == 0){
      printf("here");
      printf("%s", current_history()->line);
   }else if (strlen(cmd) == 2 && cmd[0] == '!' && isdigit(cmd[1])){
      printf("here");
   }
   else if (strlen(cmd) == 3 && cmd[0] == '!' && cmd[1] == '-' && isdigit(cmd[2])){
      printf("here");
   }
}

int main(void){
   //using_history();
   //stifle_history(HISTSIZE);

   while ( true ){
      
      char*line = readline ("\n$ ");
      if (!line || strlen(line) < 2){ continue; }

      char* delimiters = " \n";
      char* token = strtok(line, delimiters);
      int idx = 0; 
      char* command = token;
      char* arguments[128]; 
   
      while (token){
         arguments[idx++] = token;
         token=strtok(NULL, " ");
      }arguments[idx] = NULL;

      built_in_cmd_handler(command);
      //history_management(command, arguments);
      simple_cmd_handler(command, arguments);
      
      free(line);
     
   }
}