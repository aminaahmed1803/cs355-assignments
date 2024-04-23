// Author: aahmed1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include "userthread.h"

void hello_world() {
  printf("hello\n");
}

int main(void) {
  

    if (thread_libinit(PRIORITY) == -1) {
	    printf("ERROR: Could not intitialize thread library.\n");
        exit(EXIT_FAILURE);
    }


  for (int i = 0; i < 10; i++)  {
    int tid = thread_create(hello_world, NULL, 1);
    if (tid == -1){
        printf("ERROR: Could not create thread.\n");
        exit(EXIT_FAILURE);
    }
      
  }

  for (int i = 0; i < 10; i++)  {
    int tid = thread_create(hello_world, NULL, 0);
    if (tid == -1){
        printf("ERROR: Could not create thread.\n");
        exit(EXIT_FAILURE);
    }
      
  }


for (int i = 0; i < 10; i++)  {
   int tid = thread_create(hello_world, NULL, -1);
    if (tid == -1){
        printf("ERROR: Could not create thread.\n");
        exit(EXIT_FAILURE);
    }
      
  }
  for (int i = 1; i <= 30; i++)  {
    if (thread_join(i) == -1){

      printf("ERROR: Could not join thread.\n");
      exit(EXIT_FAILURE);
   
    }
     
  }

  if (thread_libterminate() == -1) {
		printf("ERROR: Could not terminate thread library.\n");
      exit(EXIT_FAILURE);
   }


  exit(EXIT_SUCCESS);
}


