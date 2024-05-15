#include <stdio.h>
#include <stdlib.h>

#include "fifo.h"
#include "sjf.h"
#include "userthread.h"

int assigned_policy;

int thread_libinit(int policy){
    assigned_policy = policy;
    switch(assigned_policy){
        case (FIFO):
            return fifo_libinit(policy);
            
        case (SJF):
            return sjf_libinit(policy);
            
        /*case (PRIORITY):
            return rr_thread_libinit(policy);*/
            
    }
    return EXIT_FAILURE;
}

int thread_libterminate(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_libterminate();
            
        case (SJF):
            return sjf_libterminate();
            
        /*case (PRIORITY):
            return rr_thread_libterminate();*/
            
    }
    return EXIT_FAILURE;
}

int thread_create(void (*func)(void *), void *arg, int priority){
    switch(assigned_policy){
        case (FIFO):
            return fifo_create(func, arg, priority);
            
        case (SJF):
            return sjf_create(func, arg, priority);
            
        /*case (PRIORITY):
            return rr_thread_create(func, arg, priority);*/
           
    }
    return EXIT_FAILURE;
}

int thread_yield(void){
    switch(assigned_policy){
        case (FIFO):
            return fifo_yield();
            
        case (SJF):
            return sjf_yield();
            
        /*case (PRIORITY):
            return rr_thread_yield();*/
            
    }
    return EXIT_FAILURE;

}

int thread_join(int tid){
    switch(assigned_policy){
        case (FIFO):
            return fifo_join(tid);
           
        case (SJF):
            return sjf_join(tid);
            
        /*case (PRIORITY):
            return rr_thread_join(tid);*/
            
    }
    return EXIT_FAILURE;
}



/*  TEST 16
  Expected:: I am func4\nI am func5\nI am func6\nI am func1\nI am func2\nI am func3\nMain exiting\n

int ret1, ret2, ret3;
void busy_wait(int n) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < 100000; j++) {
    }
  }
}

void func1(void *arg) {
  busy_wait(3);
  thread_join(ret1);
  printf("I am func1\n");
}

void func2(void *arg) {
  busy_wait(2);
  thread_join(ret2);
  printf("I am func2\n");
}

void func3(void *arg) {
  busy_wait(1);
  thread_join(ret3);
  printf("I am func3\n");
}

void func4(void *arg)  {	
  printf("I am func4\n");
}

void func5(void *arg) {
  printf("I am func5\n");
}

void func6(void *arg) {
  printf("I am func6\n");
}

int main() {
  thread_libinit(SJF);
  int retfirst = thread_create(func1, NULL, 0);
  int ret = thread_create(func2, NULL, 0);
  ret = thread_create(func3, NULL, 0);
  ret1 = thread_create(func4, NULL, 0);
  ret2 = thread_create(func5, NULL, 0);
  ret3 = thread_create(func6, NULL, 0);
  busy_wait(4);
  ret = thread_join(retfirst);
  printf("Main exiting\n");
  return 0;
}
*/


/* TEST 17
  Expected:: I am func1\nI am func2\nI am func3\nI am func4\nMain exiting\n
int join;
void busy_wait(int n)
{
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 100000; j++) {
		}
	}
}

void func1(void *arg)
{
	busy_wait(3);
	thread_yield();
	printf("I am func1\n");
}

void func2(void *arg)
{
	busy_wait(2);
	thread_yield();
	printf("I am func2\n");
}

void func3(void *arg)
{
	busy_wait(1);
	thread_yield();
	printf("I am func3\n");
}

void func4(void *arg) 
{	
	printf("I am func4\n");
}

int main()
{
	thread_libinit(SJF);
	int ret = thread_create(func1, NULL, 0);
	ret = thread_create(func2, NULL, 0);
	ret = thread_create(func3, NULL, 0);
	ret = thread_create(func4, NULL, 0);
	join = ret;
	busy_wait(4);
	ret = thread_join(ret);
	printf("Main exiting\n");
	return 0;
}*/


/* TEST 20
 Expected:: func1 before calling yield
            I am func1
            func2 before calling yield
            func3 before calling yield
            Main exiting
 
int join;
void busy_wait(int n)
{
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 100000; j++) {
		}
	}
}

void func1(void *arg)
{
	busy_wait(1);
	thread_yield();
	printf("func1 before calling yield\n");
	busy_wait(10);
	thread_yield();
	printf("I am func1\n");
}

void func2(void *arg)
{
	busy_wait(2);
	thread_yield();
	printf("func2 before calling yield\n");
	busy_wait(5);
	thread_yield();
	printf("I am func2\n");
}

void func3(void *arg)
{
	busy_wait(3);
	thread_yield();
	printf("func3 before calling yield\n");
	busy_wait(1);
	thread_yield();
	printf("I am func3\n");
}

int main()
{
	thread_libinit(SJF);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	ret = thread_create(func3, NULL, 0);
	join = ret;
	busy_wait(5);
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
*/


/* test 21
 Expected:: I am func1\nI am func2\nMain exiting\n

int ret1, ret2, ret3;
void busy_wait(int n)
{
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 100000; j++) {
		}
	}
}

void func1(void *arg)
{
	busy_wait(1);
	thread_yield();
	printf("I am func1\n");
}

void func2(void *arg)
{
	busy_wait(10);
	thread_yield();
	printf("I am func2\n");
}

int main()
{
	thread_libinit(SJF);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	busy_wait(5);
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
*/


/* TEST 22 FAILED
 Expected:: I am func1\nI am func1\nI am func1\nI am func1\nI am func1\nI am func1\nI am func2\nMain exiting\n
 
int ret1, ret2, ret3;
void busy_wait(int n)
{
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 100000; j++) {
		}
	}
}

void func1(void *arg)
{
	busy_wait(1);
	thread_yield();
	int i;
	for (i = 0; i < 6; i++) {
		busy_wait(1);
		printf("I am func1\n");
		thread_yield();
	}
}

void func2(void *arg)
{
	busy_wait(5);
	thread_yield();
	printf("I am func2\n");
}

int main()
{
	thread_libinit(SJF);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
*/

/* TEST 23 PASSED
  Expected:: I'm thread 1 with polling 100
  \nI'm thread 3 with polling 300
  \nI'm thread 5 with polling 500
  \nI'm thread 2 with polling 200
  \nI'm thread 4 with polling 400\n


#define THREAD_NUM 5
#define SLEEP_MS 100
#define FAILURE -1


void foo(void *arg)
{
    int num = *((int *)arg);
    int polling = SLEEP_MS * num;
    poll(NULL, 0, polling);
    if (num % 2 == 0)
    {
        thread_yield();
    }
    printf("I'm thread %d with polling %d\n", num, polling);
}

int main(int argc, char **argv)
{
    if (thread_libinit(SJF) == FAILURE)
    {
        printf("init failure\n");
        exit(EXIT_FAILURE);
    }

    // thread #
    int num[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        num[i] = i + 1;
    }

    // create threads
    int threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        threads[i] = thread_create(foo, num + i, 0);
        if (threads[i] == FAILURE)
        {
            printf("create failure\n");
            exit(EXIT_FAILURE);
        }
    }

    // join threads
    for (int i = THREAD_NUM - 1; i >= 0; --i)
    {
        if (thread_join(threads[i]) == FAILURE)
        {
            printf("join failure\n");
            exit(EXIT_FAILURE);
        }
    }

    if (thread_libterminate() == FAILURE)
    {
        printf("term failure\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}*/



/*  Nested  SJF PASSED
  Expected:: I am in the first layer haha! 0
  I am in the second layer haha! 0
  I am in the third layer haha! 0
  I am in the first layer haha! 1
  I am in the second layer haha! 1
  I am in the third layer haha! 1\n


void third_layer(void *arg)
{
    printf("I am in the third layer haha! %d\n", *(int*) arg);
    thread_yield();
}

void second_layer(void *arg)
{
    int tid = thread_create(third_layer, arg, 1);
    if (tid < 0)
    {
        exit(EXIT_FAILURE);
    }
    printf("I am in the second layer haha! %d\n", *(int*) arg);
    if (thread_join(tid) < 0)
    {
        exit(EXIT_FAILURE);
    }
}

void first_layer(void *arg)
{
    for (int i = 0; i < 2; ++i)
    {
        int tid = thread_create(second_layer, &i, 1);
        if (tid < 0)
        {
            exit(EXIT_FAILURE);
        }
        printf("I am in the first layer haha! %d\n", i);
        if (thread_join(tid) < 0)
        {
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    if (thread_libinit(SJF) == -1)
        exit(EXIT_FAILURE);

    int tid1 = thread_create(first_layer, NULL, 1);

    if (thread_join(tid1) < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (thread_libterminate() == -1)
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}*/

/*
  Expected:: I am running wisefool50!
  I am running wisefool200!
  I am running wisefool100!
  I am running wisefool300!
  I am running wisefool50!
  I am running wisefool50!
  I am running wisefool50!
  I am running wisefool50!
  I am running wisefool100!
  I am running wisefool100!
  I am running wisefool100!
  I am running wisefool100!
  I am running wisefool200!
  I am running wisefool200!
  I am running wisefool200!
  I am running wisefool200!
  I am running wisefool300!
  I am running wisefool300!
  I am running wisefool300!
  I am running wisefool300!
  Main exiting



#define N 5
#define FAIL -1


void wisefool50(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool50!\n");
    poll(NULL, 0, 50);
    thread_yield();
  }
}

void wisefool100(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool100!\n");
    poll(NULL, 0, 100);
    thread_yield();
  }
}

void wisefool200(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool200!\n");
    poll(NULL, 0, 200);
    thread_yield();
  }
}

void wisefool300(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool300!\n");
    poll(NULL, 0, 300);
    thread_yield();
  }
}

int main(void)
{
    if (thread_libinit(SJF) == FAIL)
        exit(EXIT_FAILURE);
    int tid1 = thread_create(wisefool50, NULL, 0);
    int tid2 = thread_create(wisefool200, NULL, 0);
    int tid3 = thread_create(wisefool100, NULL, 0);
    int tid4 = thread_create(wisefool300, NULL, 0);

    int num = 4;
    int tids[] = {tid1, tid2, tid3, tid4};
    for (int i = 0; i < num; i++)
    {
        if (tids[i] == FAIL)
            exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num; i++)
    {
        if (thread_join(tids[i]) == FAIL)
            exit(EXIT_FAILURE);
    }
    if (thread_libterminate() == FAIL)
        exit(EXIT_FAILURE);
    printf("Main exiting\n");
    exit(EXIT_SUCCESS);
}*/
