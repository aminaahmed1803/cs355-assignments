/**
 * random_join.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 * Description:
 * Ensure that join works despite the tid order, for all policies
 *
 * Expected Result:
 * this is a join test 3
 * this is a join test 7 
 *  (will be different after this for different policies)
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

void hello_world(void *arg){
  int test = *((int *) arg);
	printf("this is a join test %d\n", test);
}

int main(void)
{

	int test  = 7;

	if (thread_libinit(FIFO) == -1) {
		printf("ERROR: Could not intitialize thread library.\n");
      exit(EXIT_FAILURE);
   }
	
	int tid1 = thread_create(hello_world, &test, -1);
  int tid2 = thread_create(hello_world, &tid1, 0);
  int tid3 = thread_create(hello_world, &tid2, 1);
  int tid4 = thread_create(hello_world, &tid3, 0);
  int tid5 = thread_create(hello_world, &tid4, 1);
  int tid6 = thread_create(hello_world, &tid5, 1);
  int tid7 = thread_create(hello_world, &tid6, -1);

  thread_join(tid4);
  thread_join(tid7);
  thread_join(tid6);
  thread_join(tid5);

	if (thread_libterminate() == -1) {
		printf("ERROR: Could not terminate thread library.\n");
      exit(EXIT_FAILURE);
   }

	return 0;
}