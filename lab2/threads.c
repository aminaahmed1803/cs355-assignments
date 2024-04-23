#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

struct argumentData{
   char* argument;
};

void print_arg(char* data){
   struct timeval start = {4, 2};
   unsigned int seed = gettimeofday(&start ,0);
   int r = (rand_r(&seed) % 10); 
   char* arg = data;


   for (int i=0 ; i < r ; i++){
      printf("%s ", arg);
   }
   printf("\n");
}

int main(int argc, char* argv[]) {
   
   if (argc < 2) {
      printf("usage: <arguments>\n");
      exit(0);
   }

   pthread_t threaded_args[argc-1] ;

   for (int i=0 ; i<argc-1 ; i++){
      struct argumentData temp = {argv[i+1]};
      //pthread_create(&threaded_args[i], NULL, (void*) print_arg, (void*) &temp);
      pthread_create(&threaded_args[i], NULL, (void*) print_arg, (void*) argv[i+1]);
   }

   for (int i=0 ; i<argc-1 ; i++){
      pthread_join(threaded_args[i], NULL);
   }
}