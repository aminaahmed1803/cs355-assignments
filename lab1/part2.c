/*
	File:	kt_frame.c
	Frame for the kernel timer lab exercise
*/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>

#define INTERVAL_SECS 		1
#define INTERVAL_MICROSECS 	0
#define VALUE_SECS 		1
#define VALUE_MICROSECS 	0
#define	MICROS 			1000000

int realSeconds;

void setrtimer(struct itimerval *);
void catchsig(int);
void showtime(struct itimerval *);
void age(const struct timeval *);

int main(int argc, char **argv) {
  struct itimerval realt;
  struct timeval start;
 
  if (argc > 1) {
    printf("Usage: %s takes no arguments, ctrl-c to quit\n", argv[0] );
    exit(0);
  }
  
  gettimeofday(&start, NULL);
  setrtimer(&realt);
  while(1) {
    pause();
    showtime(&realt);
    age(&start);
  }
  return 0;
}

/*
  Initialize the ITIMER_REAL interval timer.
  Its interval is one second.  Its initial value is one second.
*/
void setrtimer(struct itimerval *ivPtr) {
   
}

/*
  Define a signal handler for a real interval timer.
  Increment the global realSeconds;
  Reset the handler for SIGALRM.
*/
void catchsig(int sig) {
}

/*
  Show elapsed time computed with an interval timer
 */
void showtime(struct itimerval *itvPtr ) {
}

/*
  Display age in seconds and microseconds of the program.
  Use gettimeofday() from OS.
 */
void age( const struct timeval *startPtr ) {
    struct timeval now;
    long secs, microsecs;

    gettimeofday(&now, NULL );
    secs = now.tv_sec - startPtr->tv_sec;
    microsecs = now.tv_usec - startPtr->tv_usec;
    if ( microsecs < 0 ) {
	microsecs += MICROS;
	secs--;
    }
    printf( "\tTotal time: %ld.%ld secs\n", secs, microsecs );
}
