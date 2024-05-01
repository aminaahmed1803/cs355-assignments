/*
random 0 100 0
random 0 1000 1
random 0 100000 1
random 0 500000 1
random 0 2000000 0
*/
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("Usage: %s sd # 0/1\nsd is a seed used for srand()\n# specifies the number of tests\n0/1 specifies False/True for enabling comparison with malloc\n", argv[0]);
    exit(0);
  }

  // psudeo-random seed
  unsigned int seed = atoi(argv[1]);
  
  // number of operations to perform
  long long n = atoll(argv[2]);

  // if true, write data into allocated memory
  bool writeData = atoi(argv[3]);

  // maximum number of concurrent allocations to track
  int max_allocs = 1000;

  // size for allocation request
  int min_alloc_size = 1;
  int max_alloc_size = 128;
  
  // allowed constant overhead
  int slack = 32;

  // max header size per allocation
  int header_size = 32;

  // request size up to 64+32, header up to 32 bytes
  int max_chunk_size = max_alloc_size + header_size;

  // most possible space, no more than max_allocs+1 unusable free chunks
  int region_size = max_allocs * max_chunk_size * 2 + max_chunk_size;

  void** ptr = calloc(sizeof(void*), max_allocs);
  int* size = calloc(sizeof(int), max_allocs);
  void** shadow = calloc(sizeof(void*), max_allocs);

  /*******************************************************************
   Please note that random() gives psudeo-random, not true random data.
   If the seed is set to the same value, the sequence generated by
   random() will be the same. Using psuedo-random number generators is
   a common testing technique.
  *******************************************************************/
  srandom(seed);

  assert(Mem_Init(region_size + slack) == 0);

  int slot;
  bool doAlloc;
  //bool doWrite;

  long long i;


  for (i=0; i<n; i++) {
    slot = random() % max_allocs;
    doAlloc = random() % 4;
    //doWrite = writeData;
    
    if (!doAlloc || ptr[slot] != NULL) {
      assert(Mem_Free(ptr[slot], 1) == 0);
      //assert(Mem_Free(ptr[slot], 2) == 0);
      free(shadow[slot]);
      ptr[slot] = NULL;
      shadow[slot] = NULL;
    }

    //printf("Iteration %lld\n", i);
    //Mem_Dump();
    if (doAlloc) {
      size[slot] = min_alloc_size + (random() % (max_alloc_size - min_alloc_size + 1));
      ptr[slot] = Mem_Alloc(size[slot]);
      assert(ptr[slot] != NULL);
      if (writeData) {
        shadow[slot] = malloc(size[slot]);
	      int j;
	      for (j=0; j<size[slot]; j++) {
	        char data = random();
	        *((char*)(ptr[slot] + j)) = data;
	        *((char*)(shadow[slot] + j)) = data;
	      }
      }
    }
    
  }
  //assert(Mem_Free(NULL, 1) == 0);
  
  if (writeData) {
    for (slot=0; slot<max_allocs; slot++) {
      if (ptr[slot] != NULL) {
	      assert(memcmp(ptr[slot], shadow[slot], size[slot]) == 0);
      }
    }
  }
 
  
  exit(0);
}