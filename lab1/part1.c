#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
 
void signal_handler(int sig)
{
   if (sig == SIGUSR1)
      printf("parent has received signal: %d\n", sig);
   if (sig == SIGUSR2)
      printf("child has received signal: %d\n", sig);

}


int main(void)
{
   /* Install a signal handler. */
   pid_t pid; 
   int status = 0;
   signal(SIGUSR1, signal_handler);
   signal(SIGUSR2, signal_handler);
   
   pid = fork();

   if (pid == 0) /*code for child*/ {
      kill(getppid(), SIGUSR1);
      sleep(5);
   } else if (pid > 0) /*code for parent*/ {
      kill(pid, SIGUSR2);
      sleep(1);
      kill(pid, SIGTERM);
      wait(&status);
      printf("Child killed\n");
   } else {
      printf("Fork returned error code\n");
   }
   return 0;
}
