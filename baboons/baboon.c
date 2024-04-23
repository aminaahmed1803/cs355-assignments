/**
1. Simulate each baboon as a separate process. You will want to set up shared memory regions with mmap and/or shm open.
2. Altogether, 100 baboons will cross the canyon, with a random number generator speci-fying whether they are eastward moving 
or westward moving (with equal probability).
3. Use a random number generator, so the time between baboon arrivals is between 1 and 6 seconds. 
You should launch all the baboons together, but have each sleep a random number of 1 to 6 seconds, 
instead of generating each sequentially with 1 to 6 second delay. The former will force a lot more crowding 
and be more interesting.
4. Each baboon takes 1 second to get on the rope. (That is, the minimum inter-baboon spacing is 1 second.)
5. All baboons travel at the same speed. Each traversal takes exactly 4 seconds, after the baboon is on the rope.
6. Use semaphores for synchronization.
7. You should generate sufficient printouts to demonstrate correctness of program, which should include:
– No east/west collision on rope
– No collision getting onto the rope
– There are mulitple baboons on the rope at a time
– Maximum number of baboons on rope is not exceeded 
– Direction switching for the no-starvation version (40)
At the minimum, the following events should be logged. You might need more: 
    – west/east baboon [ID] arrived [system time]
    – west/east baboon [ID] starts crossing [system time]
    – west/east baboon [ID] finishes crossing [system time]
Turn in your code electronically (via submit and use -p 3). Also turn in hardcopies of three test-run results in class. 
If the test-runs are long, please cut out the middle so that you do not print out too many pages. 
Include enough run results so that it clearly shows how your program works. And make sure you mark your test-runs 
starvation or non-starvation.
*/
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

enum direction { WEST=0, EAST=1 };
sem_t mutex;
sem_t blocked[2];
int blockedCnt[2];
int travellers[2];

void innit(){
    sem_init(&mutex, 1, 1); //  A non-zero value means the semaphore is shared between processes and a value of zero means it is shared between threads.
    sem_init(&blocked[WEST], 1, 1);
    sem_init(&blocked[EAST], 1, 1);

}

/*Chap 6 Question 39 of MOS 4th ed TANENBAUM*/
void baboon(enum direction dir){
    int revdir = !dir;
    sem_wait(&mutex); // p is semwait mutex->P(); down
    while (travellers[revdir]) {
        blockedCnt[dir]++; // announce our intention to block
        sem_post(&mutex); //->V(); // trade mutex for block
        sem_wait(&blocked[dir]); //->P();
        sem_wait(&mutex); // ->P();
    }
    travellers[dir]++; // we’re free to cross

    sem_post(&mutex); 
    // cross bridge
    sem_wait(&mutex);

    travellers[dir]--;
    if (!travellers[dir]) {
        // if we’re the last one heading this way,
        // wakeup baboons waiting for us to finish.
        while(blockedCnt[revdir]--);
            sem_post(&blocked[revdir]); //->V();
    }
    sem_post(&mutex);
}

/*Chap 6 Question 40 of MOS 4th ed TANENBAUM*/
void baboon_avoid_starvation(enum direction dir){
    void;
}


void main() {

}