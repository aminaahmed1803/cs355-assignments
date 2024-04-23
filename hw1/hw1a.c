#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

unsigned long prev_total_interrupt_count = 0;
unsigned long total_interrupt_count = 0;

/*
 * Function:  read_proc
 * --------------------
 * reads from /proc/interrupts 
 * and prints the number of interrupts
 * in the previous second and the 
 * number of interrupts since 
 * boot time.
 
 *  returns: void
 */
void read_proc(){
   
   FILE *file = fopen("/proc/interrupts", "r");   
   if (file == NULL)
        exit(EXIT_FAILURE);

   char * line = NULL;
   size_t len = 0;
   ssize_t read;
   unsigned long current_total_interrupts = 0;
   
   while ((read = getline(&line, &len, file)) != -1) {
      char * token = strtok(line, "          ");
      
      while( token != NULL ) {
          //printing each token
         current_total_interrupts += atoi(token);
         token = strtok(NULL, " ");
      }  
   }

   prev_total_interrupt_count = total_interrupt_count;
   total_interrupt_count = current_total_interrupts;

   printf("1. previous second: %lu, 2. boot time: %lu\n", total_interrupt_count-prev_total_interrupt_count, total_interrupt_count);

   if (line)
      free(line);
   
   fclose(file);
}
 
/*alarm handler*/
void alarm_handler(int sig)
{
   read_proc();
   alarm(1);
}

int main(void)
{
   /* Install a signal handler. */
   signal(SIGALRM, alarm_handler);
   
   alarm(1);

   while(1) {
      pause();
   }
}