/**
 * Author: aahmed1
 * Date: 3/24/2024
 * 
 * Description:
 * gives meminit invalid args
 *
 * Expected Result:
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <assert.h>


#include "mem.h"

#define N 36

int main() {

    if (Mem_Init(-1) == 0) {
        printf("ERROR: Mem_Init took negative arg\n");
        exit(EXIT_FAILURE);
    }

    if (Mem_Init(1024) == -1) {
        printf("ERROR: Could not Mem_Init\n");
        exit(EXIT_FAILURE);
    }

    if (Mem_Init(1024) == 0) {
        printf("ERROR: Mem_Init initialized twice\n");
        exit(EXIT_FAILURE);
    }

    if (Mem_Init(-1) == 0) {
        printf("ERROR: Mem_Init took invalid args\n");
        exit(EXIT_FAILURE);
    }

    assert(m_error == E_BAD_ARGS);

    return 0;
}