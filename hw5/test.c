#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"
#define HEADER (32)
#define SLACK (32)
#define FAIL -1
#define TRUE 1
/* check first pointer returned is 8-byte aligned */
int align() {
   assert(Mem_Init(4096) == 0);
   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   uintptr_t addr = (uintptr_t)ptr;
   assert(addr % 8 == 0);
   return 0;
}
/* a few allocations checked for alignment */
int align2() {
   assert(Mem_Init(4096) == 0);
   void* ptr[4];
   ptr[0] = Mem_Alloc(8);
   ptr[1] = Mem_Alloc(16);
   ptr[2] = Mem_Alloc(32);
   ptr[3] = Mem_Alloc(8);
   assert((uintptr_t)(ptr[0]) % 8 == 0);
   assert((uintptr_t)(ptr[1]) % 8 == 0);
   assert((uintptr_t)(ptr[2]) % 8 == 0);
   assert((uintptr_t)(ptr[3]) % 8 == 0);
   return 0;
}
/* many odd sized allocations checked for alignment */
int align3() {
   assert(Mem_Init(4096) == 0);
   void * ptr[9];
   ptr[0] = Mem_Alloc(1);
   ptr[1] = (Mem_Alloc(5));
   ptr[2] = (Mem_Alloc(14));
   ptr[3] = (Mem_Alloc(8));
   ptr[4] = (Mem_Alloc(1));
   ptr[5] = (Mem_Alloc(4));
   ptr[6] = (Mem_Alloc(9));
   ptr[7] = (Mem_Alloc(33));
   ptr[8] = (Mem_Alloc(55));
   assert((uintptr_t)(ptr[0]) % 8 == 0);
   assert((uintptr_t)(ptr[1]) % 8 == 0);
   assert((uintptr_t)(ptr[2]) % 8 == 0);
   assert((uintptr_t)(ptr[3]) % 8 == 0);
   assert((uintptr_t)(ptr[4]) % 8 == 0);
   assert((uintptr_t)(ptr[5]) % 8 == 0);
   assert((uintptr_t)(ptr[6]) % 8 == 0);
   assert((uintptr_t)(ptr[7]) % 8 == 0);
   assert((uintptr_t)(ptr[8]) % 8 == 0);
   return 0;
}
/* a simple 8 byte allocation */
int alloc() {
   assert(Mem_Init(4096) == 0);
   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   return 0;
}
/* a few aligned allocations */
int alloc2() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Alloc(8) != NULL);
   assert(Mem_Alloc(16) != NULL);
   assert(Mem_Alloc(32) != NULL);
   assert(Mem_Alloc(8) != NULL);
   return 0;
}
/* many odd sized allocations */
int alloc3() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Alloc(1) != NULL);
   assert(Mem_Alloc(5) != NULL);
   assert(Mem_Alloc(14) != NULL);
   assert(Mem_Alloc(8) != NULL);
   assert(Mem_Alloc(1) != NULL);
   assert(Mem_Alloc(4) != NULL);
   assert(Mem_Alloc(9) != NULL);
   assert(Mem_Alloc(33) != NULL);
   assert(Mem_Alloc(55) != NULL);
   return 0;
}
/* bad arguments to Mem_Init */
int badinit() {
   assert(Mem_Init(0) == -1);
   assert(m_error == E_BAD_ARGS);
   return 0;
}
/* bad arguments to Mem_Init */
int badinit2() {
   assert(Mem_Init(-4096) == -1);
   assert(m_error == E_BAD_ARGS);
   return 0;
}
/* check for coalesce free space */
int coalesce() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);
   while (Mem_Alloc(800) != NULL)
      ;
   assert(m_error == E_NO_SPACE);
   assert(Mem_Free(ptr[1], 1) == 0);
   assert(Mem_Free(ptr[2], 1) == 0);
   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);
   return 0;
}
/* check for coalesce free space */
int coalesce2() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);
   while (Mem_Alloc(800) != NULL)
      ;
   assert(m_error == E_NO_SPACE);
   assert(Mem_Free(ptr[2], 1) == 0);
   assert(Mem_Free(ptr[1], 1) == 0);
   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);
   return 0;
}
/* check for coalesce free space */
int coalesce3() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);
   while (Mem_Alloc(800) != NULL)
      ;
   assert(m_error == E_NO_SPACE);
   assert(Mem_Free(ptr[1], 1) == 0);
   assert(Mem_Free(ptr[3], 1) == 0);
   assert(Mem_Free(ptr[2], 1) == 0);
   ptr[2] = Mem_Alloc(2400);
   assert(ptr[2] != NULL);
   return 0;
}
/* check for coalesce free space */
int coalesce4() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);
   while (Mem_Alloc(800) != NULL)
      ;
   assert(m_error == E_NO_SPACE);
   assert(Mem_Free(ptr[2], 0) == 0);
   assert(Mem_Free(ptr[1], 0) == 0);

   Mem_Dump();
   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] == NULL);
   assert(m_error == E_NO_SPACE);
    return 0;
}
/* check for coalesce free space (first chunk)*/
int coalesce5() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);
   while (Mem_Alloc(800) != NULL)
      ;
   assert(m_error == E_NO_SPACE);
   assert(Mem_Free(ptr[0], 1) == 0);
   assert(Mem_Free(ptr[1], 1) == 0);
   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);
   return 0;
}
/* check for coalesce free space (last chunk)*/
int coalesce6() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(880);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(880);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(880);
   assert(ptr[2] != NULL);
   ptr[3] = Mem_Alloc(880);
   assert(ptr[3] != NULL);
   assert(Mem_Alloc(880) == NULL);
   assert(m_error == E_NO_SPACE);
   // last free chunk is at least this big
   int free = (4096 - (880 + HEADER) * 4) - SLACK;
   assert(Mem_Free(ptr[3], 1) == 0);
   free += 880 + 32;
   ptr[2] = Mem_Alloc(free - HEADER);
   assert(ptr[2] != NULL);
  return 0;
}
/* call init twice */
int doubleinit() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Init(4096) == -1);
   assert(m_error == E_BAD_ARGS);
   return 0;
}
/* a simple allocation followed by a free */
int free1() {
   assert(Mem_Init(4096) == 0);
   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   assert(Mem_Free(ptr, 0) == 0);
  return 0;
}
/* a few aligned allocations and frees */
int free2() {
   assert(Mem_Init(4096) == 0);
   void* ptr[4];
   ptr[0] = Mem_Alloc(8);
   ptr[1] = Mem_Alloc(16);
   assert(Mem_Free(ptr[0], 0) == 0);
   assert(Mem_Free(ptr[1], 0) == 0);
   ptr[2] = Mem_Alloc(32);
   ptr[3] = Mem_Alloc(8);
   assert(Mem_Free(ptr[2], 0) == 0);
   assert(Mem_Free(ptr[3], 0) == 0);
   return 0;
}
/* many odd sized allocations and interspersed frees */
int free3() {
   assert(Mem_Init(4096) == 0);
   void * ptr[9];
   ptr[0] = Mem_Alloc(1);
   ptr[1] = (Mem_Alloc(5));
   ptr[2] = (Mem_Alloc(14));
   ptr[3] = (Mem_Alloc(8));
   assert(Mem_Free(ptr[1], 0) == 0);
   assert(Mem_Free(ptr[0], 0) == 0);
   assert(Mem_Free(ptr[3], 0) == 0);
   ptr[4] = (Mem_Alloc(1));
   ptr[5] = (Mem_Alloc(4));
   assert(ptr[4] != NULL);
   assert(ptr[5] != NULL);
   assert(Mem_Free(ptr[5], 0) == 0);
   ptr[6] = (Mem_Alloc(9));
   ptr[7] = (Mem_Alloc(33));
   assert(ptr[6] != NULL);
   assert(ptr[7] != NULL);
   assert(Mem_Free(ptr[4], 0) == 0);
   ptr[8] = (Mem_Alloc(55));
   assert(ptr[8] != NULL);
   assert(Mem_Free(ptr[2], 0) == 0);
   assert(Mem_Free(ptr[7], 0) == 0);
   assert(Mem_Free(ptr[8], 0) == 0);
   assert(Mem_Free(ptr[6], 0) == 0);
   return 0;
}
/* call Mem_Init with size = 1 page */
int init() {
   assert(Mem_Init(4096) == 0);
   return 0;
}
/* init that should be rounded up to 1 page */
int init2() {
   assert(Mem_Init(1) == 0);
   return 0;
}
/* second allocation is too big to fit */
int nospace() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Alloc(2048) != NULL);
   assert(Mem_Alloc(2049) == NULL);
   assert(m_error == E_NO_SPACE);
   return 0;
}
/* keep allocating 1 byte (padded to 8) until no space left */
int nospace2(void) {
  int count = 0;
  if (Mem_Init(8192) == FAIL)
    return -1;
  void *ptr;
  while (TRUE) {
    ptr = Mem_Alloc(1);
    count ++;
    if (ptr == NULL) {
      assert(count == 1025);
      return 0;
    }
  }
  return -1;
}
/* free a NULL */
int nullfree() {
   assert(Mem_Init(4096) == 0);
   assert(Mem_Free(NULL, 0) == 0);
   assert(Mem_Free(NULL, 1) == 0);
   return 1;
}
/* use worst fit free space for allocation */
int worstfit() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];
   ptr[0] = Mem_Alloc(900);
   assert(ptr[0] != NULL);
   ptr[1] = Mem_Alloc(900);
   assert(ptr[1] != NULL);
   ptr[2] = Mem_Alloc(900);
   assert(ptr[2] != NULL); 
   assert(Mem_Free(ptr[1], 0) == 0);
   ptr[1] = NULL;
   ptr[1] = Mem_Alloc(900);
   assert(ptr[1] != NULL);
   ptr[3] = Mem_Alloc(1100);
   assert(ptr[3] == NULL);
   assert(m_error == E_NO_SPACE);
   return 1;
}
/* write to a chunk from Mem_Alloc */
int writeable() {
   assert(Mem_Init(4096) == 0);
   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   *((int*)ptr) = 42;   // check pointer is in a writeable page
   return 1;
}

int main(){

   //align();
    //align2();
    //align3();
    //alloc();
    //alloc2();
    //alloc3();
    //badinit();
    //badinit2();
    //worstfit(); //FAILED
    //coalesce();
    //coalesce2();
    //coalesce3();
    //coalesce4(); //FAILED
    //coalesce5(); 
    //coalesce6(); 
    //doubleinit();
    //badinit2();
    //free1();
    //free2();
    //free3();
    //init();
    //init2();
    //nospace(); 
    //nullfree();
    //writeable();
    exit(0);
}