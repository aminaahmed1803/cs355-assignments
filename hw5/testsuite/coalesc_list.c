/**
 * Author: aahmed1
 * Date: 3/24/2024
 *
 * Expected Result:
 * The first memory dump should display fragmented memort
 * The second memory dumb should display chunks of free memory
 * between allocated chunks of memory 
 * The last memory dump should display one chunk of free memory
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include "mem.h"

#define BYTE 8
#define N 10

int main() {
    // Initialize memory alloc
    if (Mem_Init(500) == -1) {
        printf("ERROR: Could not Mem_Init\n");
        exit(EXIT_FAILURE);
    }

    void *ptr[N];

    for (int i=0; i<N; i++){
        ptr[i] = Mem_Alloc(BYTE);
    }
    
    Mem_Dump();

    for (int i=0; i<N; i++){
        if (i%3 != 0 ){
            Mem_Free(ptr[i], 0);
        }
    }

    Mem_Free(ptr[9], 1);

    Mem_Dump();

    Mem_Free(ptr[0], 0);
    Mem_Free(ptr[3], 0);
    Mem_Free(ptr[6], 1);

    Mem_Dump();

    return 0;
}