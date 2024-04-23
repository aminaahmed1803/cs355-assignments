#ifndef COMMON_H
#define COMMON_H

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "mem.h"
#include <errno.h>
#include <stdint.h>


#define FAIL -1
#define SUCCESS 0 
#define TRUE 1
#define FALSE 0 
#define ALIGN 7
#define HEADER_SIZE 32 
#define MIN_SIZE 48
#define INITIALIZE 0


// total_size = HEADDER SIZE + PADDING + size

// for each block of memory
typedef struct _head {
    uint32_t free;       // 4 bytes
    uint32_t size;       // memory size
    struct _head *next;  // 8 bytes 
    struct _head *prev;  // 8 bytes
    struct _head *next_free;  // 8 bytes
} HEAD;


#endif