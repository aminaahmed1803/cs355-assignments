
/*
should finish in order 3 -> 2 -> 1
since 3 has priority -1, 2 has priority 0, and 1 has priority 1
*/

void f1(){
    for (int i=0; i<3 ; i++){
    printf("in thread one \n");
    sleep(1);
  }
}

void f2(){
    for (int i=0; i<3 ; i++){
    printf("in thread two \n");
    sleep(1);
  }
}

void f3(){
    for (int i=0; i<3 ; i++){
    printf("in thread three \n");
    sleep(1);
  }
}

int main(){

    thread_libinit(PRIORITY);
    
    int i3= thread_create(f3, NULL,1);
    int i2= thread_create(f2, NULL, 0);
    int i1= thread_create(f1, NULL, -1);
    thread_join(i1);
    
    
    thread_libterminate();
    return 0;
}