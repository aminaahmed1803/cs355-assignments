/**
 * Author: aahmed1
 * Date: 3/24/2024
 *
 * Expected Result:
 * The memory dump should display
 * chunks of memory that are 
 * eight bit aligned 
 * meaning they are factors of 8
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include "mem.h"

int main() {
    // Initialize memory alloc
    if (Mem_Init(1024) == -1) {
        printf("ERROR: Could not Mem_Init\n");
        exit(EXIT_FAILURE);
    }

    void *ptr1 = Mem_Alloc(101);
    void *ptr2 = Mem_Alloc(4);
    void *ptr3 = Mem_Alloc(5);
    void *ptr4 = Mem_Alloc(981);
    
    Mem_Dump();

    Mem_Free(ptr1, 0);
    Mem_Free(ptr2, 0);
    Mem_Free(ptr3, 0);
    Mem_Free(ptr4, 1);

    return 0;
}