/**
 * test_priority.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 * Description:
 * ensure that jobs with higher prioirities finish first
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "userthread.h"

#define N 4

void task(void* args) {
    int* info = (int*)args;
    int p = (int)info[0];
    int num = (int)info[1];
  
    sleep(2);
    printf("Task %d, %d finished.\n", num, p);
}

int main(void) {
    if (thread_libinit(PRIORITY) == -1) {
        exit(EXIT_FAILURE);
    }

    int info[][2] = {{-1, 1}, {0, 2},  {1, 3}, {0, 5}};

    
    int tids[N];

    for (int i = 0; i < N; i++) {
        tids[i] = thread_create(task, &info[i], info[i][0]);
        if (tids[i] == -1) {
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N; i++) {
        if (thread_join(tids[i]) == -1) {
            exit(EXIT_FAILURE);
        }
    }

    if (thread_libterminate() == -1) {
        printf("Failed to terminate thread library\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}