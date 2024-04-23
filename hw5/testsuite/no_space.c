/**
 * Author: aahmed1
 * Date: 3/24/2024
 * 
 * Description:
 * allocates space until there is no more left
 *
 * Expected Result:
 * it should end. if there is an infited loop, 
 * something is wrong. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define FAIL -1
#define BYTE 8


int main() {
    if (Mem_Init(getpagesize()) == FAIL) {
        printf("Can't init\n");
        exit(EXIT_FAILURE);
    }
    
    // this alloc that should fail
    void *ptr = Mem_Alloc(BYTE);

    while (ptr != NULL){
        printf("allocated 8 bytes\n");
        ptr = Mem_Alloc(BYTE) ;
    }
    
    // check if alloc fails
    if (ptr == NULL && m_error == E_NO_SPACE) {
        printf("No space left to allocate -- expected\n");
    }
    
    return 0;
}