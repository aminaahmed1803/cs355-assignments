/**
 * sjf_stress.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 * Description: this shows the sjp ens
 *
 *
 * Expected Output:
 * in thread one 
 * in thread one 
 * in thread one 
 * in thread three
 * in thread three
 * in thread two 
 * in thread two 
 * in thread two 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "userthread.h"

void make_thread_three()
{
	 for (int i=0; i<2 ; i++){
    printf("in thread three\n");
    sleep(0.5);
  }

}

void make_thread_two()
{
   
	 for (int i=0; i<3 ; i++){
    printf("in thread two \n");
    sleep(2);
  }
}


void make_thread_one()
{
  for (int i=0; i<3 ; i++){
    printf("in thread one \n");
    sleep(0.25);
  }
}

int main(void)
{
	if (thread_libinit(SJF) == -1) {
		printf("ERROR: Could not intitialize thread library.\n");
      exit(EXIT_FAILURE);
   }

	int tid = thread_create(make_thread_one, NULL, 0);
  thread_create(make_thread_two, NULL, 0);
  thread_create(make_thread_three, NULL, 0);


	 thread_join(tid);

	if (thread_libterminate() == -1) {
		printf("ERROR: Could not terminate thread library.\n");
      exit(EXIT_FAILURE);
   }

	return 0;
}
