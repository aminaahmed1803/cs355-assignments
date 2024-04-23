#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

/*signal handler*/
void signal_handler(int sig)
{ 
   printf("Help! I think I’ve been shot!!!\n");
   exit(0);
}

/*
 * Function:  readFile
 * --------------------
 * reads from file using
 * c system commands
 * and prints them
 */
void readFile (char *fn)
{
   int file_size;     // Number of bytes returned
   char* data[128];    // Retrieved data
   int num_bytes = 128;   // Number of bytes to read

   int fd = open (fn, O_RDONLY);
   if (fd == -1) {
      fprintf (stderr, "%s() error: file open failed '%s'.", __func__, fn);
      return;
   }

   while (file_size = read(fd, data, num_bytes) > 0){
      write( 0, data, num_bytes );
      memset(data, '\0', num_bytes);
   }
   
   close(fd);
}

/*
 * Function:  readFile
 * --------------------
 * reads from stdin using
 * c system commands
 * and prints them
 */
void readStdin(){
   
   char* data[128]; 
   int num_bytes = 128; 
   int bytes_read;

   while(1){
      bytes_read = read(1 , data ,  num_bytes);
      write(1, data ,bytes_read);
      memset(data, '\0', num_bytes);
   }
}

int main (int argc, char **argv) {
   
   bool hyphen = false; 
   /*register a range of signals*/
   for (int sig = 1; sig <= NSIG; sig++) {
      signal(sig, signal_handler);
   }

   /*reads file from command line*/
   if (argc > 1){
      for( int i=1 ; i<argc ; i++){
         if (strcmp("-", argv[i]) == 0){
            hyphen = true;
            break;
         } 
         printf("\n");
         readFile(argv[i]);
         printf("\n");
      }
   }
   if (!hyphen && argc > 1){ exit(0); }

   /*reads file from stdin*/
   readStdin();
   
   return 0;
}
