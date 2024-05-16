/**
 * sjf_schedule.c
 * Author: aahmed1
 * Date: 3/8/2024
 * 
 *
 * 
 * Expected:
 * func 1 finished
 * func 2 finished
 * func 3 finished
*/

int f1(){
    sleep(1);
    thread_yield();
    printf("func 1 finished\n");
}


int f2(){
    sleep(2);
    thread_yield();
    printf("func 2 finished\n");
}



int f3(){
    sleep(3);
    thread_yield();
    printf("func 3 finished\n");
}

int main(){
    thread_libinit(SJF);
    
    int i1= thread_create(f3, NULL, 0);
    int i2= thread_create(f2, NULL, 0);
    int i3= thread_create(f1, NULL, 0);
    thread_join(i1);
    
    
    thread_libterminate();
    return 0;
}