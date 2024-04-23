/**
 * Author: aahmed1
 * Date: 3/24/2024
 * 
 * Description:
 * checks that memory can be written to
 * and read from after allocation
 *
 * Expected Result:
 * Hello World!
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include "mem.h"

#define N 36

int main() {
    // Initialize memory alloc
    if (Mem_Init(1024) == -1) {
        printf("ERROR: Could not Mem_Init\n");
        exit(EXIT_FAILURE);
    }

    char **ptr = (char **)Mem_Alloc(N);
    if (ptr == NULL){
        printf("ERROR: Could not Mem_Alloc\n");
        exit(EXIT_FAILURE);
    }

    *ptr = "Hello World!";
    printf("%s\n", *ptr);

    if (Mem_Free(ptr, 1) == -1 ){
        printf("ERROR: Could not Mem_Free\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}