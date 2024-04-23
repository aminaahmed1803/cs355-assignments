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
void signal_handler(int sig)
{
   read_proc();
}

int main(int argc, char* argv[]) {

   if (argc < 2){
      printf("usage: %s -s <seconds>\n", argv[0]);
      exit(0);
   }

   int opt;
   int seconds; 
   
   while ((opt = getopt(argc, argv, ":s:")) != -1) {
    switch (opt) {
      case 's': seconds = atoi(optarg); break;
      case '?': printf("usage: %s -s <seconds>\n", argv[0]); break;
    }
  }
   
   /*time to count down seconds*/
   signal(SIGALRM, signal_handler);
   struct itimerval it_val;
   it_val.it_value.tv_sec = seconds;
   it_val.it_value.tv_usec = 0;  
   it_val.it_interval = it_val.it_value;
   if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
      perror("setitimer");
      exit(1);
   }
   while(1);
}