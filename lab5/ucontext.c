#include <ucontext.h> 
#include <sys/types.h> 
#include <signal.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#define STACKSIZE 4096 

ucontext_t uc;
ucontext_t main_uc;


void context_f(){
  printf("context: Hello World\n");
  swapcontext(&uc, &main_uc);
}

void main_f(){
  printf("main: Hello World\n");
  //swapcontext(&main_uc, &uc);
}

int main(){

   void *stack;
   void *main_stack;

   getcontext(&uc);
   getcontext(&main_uc);

   stack = malloc(STACKSIZE);
   main_stack = malloc(STACKSIZE);


   uc.uc_stack.ss_sp = stack;
   uc.uc_stack.ss_size = STACKSIZE;
   uc.uc_stack.ss_flags = 0;
   sigemptyset(&(uc.uc_sigmask));
   uc.uc_link = NULL;
  
   main_uc.uc_stack.ss_sp = main_stack;
   main_uc.uc_stack.ss_size = STACKSIZE;
   main_uc.uc_stack.ss_flags = 0;
   sigemptyset(&(main_uc.uc_sigmask));
   main_uc.uc_link = NULL;

   makecontext(&uc, context_f, 0, 0);
   makecontext(&main_uc, main_f, 0, 0);

   swapcontext(&main_uc, &uc);

   while(1){
      main_f();
      swapcontext(&main_uc, &uc);      
   }

  return 0;
}
