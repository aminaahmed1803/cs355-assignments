/* use worst fit free space for allocation */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
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

   exit(0);
}
