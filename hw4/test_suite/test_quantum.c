/**
 * test_quantum.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 * Description:
 * Tests premption. The only way for thread 2 to enter is
 * if thread 1 is stopped and swapped with thread 2 
 *
 * Expected Result:
 * Entered thread 1
 * Created thread 2
 * Entered thread 2
 *
 * the userthread_log.txt should show that thread one 
 * should be switched for thread two after 100 milliseconds of 
 * its running. The thread two should run indefinitley with it eventually 
 * getting a priority of 1 and an increasing quantum size from 
 * around 100 ms to 200 ms to 300 ms as shown
[0]	CREATED	1	-1
[0]	SCHEDULED	1	-1
[0]	CREATED	2	0
[1]	FINISHED	1	-1
[1]	SCHEDULED	2	0
[205]	STOPPED	2	0
[205]	SCHEDULED	2	1
[511]	STOPPED	2	1
[511]	SCHEDULED	2	1
[814]	STOPPED	2	1
[814]	SCHEDULED	2	1
 */
#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"


void thread_two_print()
{
	printf("Entered thread 2\n");
	
   while(1);
   
}

void thread_one_print()
{
	printf("Entered thread 1\n");
	
   int tid = thread_create(thread_two_print, NULL, -1);
	if (tid == -1) {
      printf("ERROR: Could not create thread.\n");
      exit(EXIT_FAILURE);
   }

	printf("Created thread 2\n");

	// The only way for thread 2 to be entered is 
	// if thread 1 is switched for  thread 2.
	
}

int main(void)
{
	if (thread_libinit(PRIORITY) == -1) {
		printf("ERROR: Could not intitialize thread library.\n");
      exit(EXIT_FAILURE);
   }

	int tid = thread_create(thread_one_print, NULL, -1);
	if (tid == -1) {
      printf("ERROR: Could not create thread.\n");
      exit(EXIT_FAILURE);
   }

	if (thread_join(tid) == -1) {
      printf("ERROR: Could not join thread.\n");
      exit(EXIT_FAILURE);
   }

	if (thread_libterminate() == -1) {
		printf("ERROR: Could not terminate thread library.\n");
      exit(EXIT_FAILURE);
   }

	return 0;
}