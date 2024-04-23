/**
 * hello_thread.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 * Description:
 * tests that each thread is logged as expected. 
 * Ensure that userthread_log.txt is not empty to test truncate
 *
 * Expected Result:
[0]	CREATED	1	-1
[0]	CREATED	2	1
[0]	CREATED	3	0
[0]	SCHEDULED	2	1
[1000]	FINISHED	2	1
[1000]	SCHEDULED	1	-1
[2000]	FINISHED	1	-1
[2000]	SCHEDULED	3	0
[3000]	FINISHED	3	0
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "userthread.h"

void test(){
  sleep(1);
}

int main(void) {
  

    if (thread_libinit(PRIORITY) == -1) {
	    printf("ERROR: Could not intitialize thread library.\n");
        exit(EXIT_FAILURE);
    }


  
    int tid1 = thread_create(test, NULL, -1);
    int tid2 = thread_create(test, NULL, 1);
    int tid3 = thread_create(test, NULL, 0);

  
    thread_yield();
    thread_join(tid3);
    thread_yield();

  if (thread_libterminate() == -1) {
		printf("ERROR: Could not terminate thread library.\n");
      exit(EXIT_FAILURE);
   }


  exit(EXIT_SUCCESS);
}
