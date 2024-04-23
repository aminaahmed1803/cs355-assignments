/**
 * Author: aahmed1
 * Date: 3/24/2024
 * 
 * Description:
 * allocates two million times with
 * calls to mem_free having either 
 * 0, 1, or 2 as coalesc params
 *
 * Expected Result:
 * 0.05-0.07
 */

#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>

#define NUM_ALLOC 2000000
#define FREE_FREQ 1000          
#define LIST_CO 10000  
#define BYTE 8

clock_t begin, end;

static void print_execution_time(clock_t begin, clock_t end) {
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Execution time: %.2f seconds\n", time_spent);
}

int main() {
  
    int result = Mem_Init(NUM_ALLOC * 40);
    assert(result == 0);

    void **ptrs = malloc(sizeof(void*) * NUM_ALLOC);
    if (ptrs == NULL){
        printf("malloc failed\n");
        return 0;
    }

    for (int i = 0; i < NUM_ALLOC; i++) {
        ptrs[i] = Mem_Alloc(BYTE);
        assert(ptrs[i] != NULL);

        if (i % FREE_FREQ == FREE_FREQ - 1){
            
            if (i % LIST_CO == 1 ){
                Mem_Free(ptrs[i-FREE_FREQ+1], 1);
            }
            else if (i % LIST_CO == 2 ){
                
                Mem_Free(ptrs[i-FREE_FREQ+1], 2);
            }
            else 
                Mem_Free(ptrs[i-FREE_FREQ+1], 0);
        }
           

    }
    
    end = clock();
    print_execution_time(begin, end);
    free(ptrs);
    return EXIT_SUCCESS;
}