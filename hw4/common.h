#ifndef __COMMON_H_ 
#define __COMMON_H_ "common.h"

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <ucontext.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define ALPHA 0.5
#define AVERAGE_BURST 1000
#define NEG_PRIORITY -1
#define ZERO_PRIORITY 0
#define ONE_PRIORITY 1
#define NEG_WEIGHT 9
#define ZERO_WEIGHT 6
#define ONE_WEIGHT 4
#define MILLISEC 1000
#define EXIT_SUCCESS 0
#define FAILURE -1
#define STACKSIZE (256*1024)
#define INITIALIZE_VALUE 0
#define ONE_INTERVAL 300
#define ZERO_INTERVAL 200
#define NEG_INTERVAL 100
#define INTERVAL 100
#define DEFAULT_RUNTIME 50

enum {
	READY,
	RUNNING,
	ZOMBIE,
} STATUS;

#endif