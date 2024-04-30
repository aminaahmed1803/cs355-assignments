/* keep allocating 1 byte (padded to 8) until no space left */
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define FAIL -1
#define TRUE 1

int main(void) {
  int count = 0;
  if (Mem_Init(8192) == FAIL)
    exit(EXIT_FAILURE);
  void *ptr;
  while (TRUE) {
    ptr = Mem_Alloc(1);
    count ++;
    if (ptr == NULL) {
      assert(count == 1025);
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_FAILURE);
}
