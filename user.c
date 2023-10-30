#include <stdio.h>
#include <pthread.h>
#include "profiler.h"



void* thread_function(void *arg) {
    sleep(10);
    return NULL;
}



int main(){
    pthread_t tid;
    pthread_create(&tid, NULL, thread_function, NULL);
    gprof_init(tid);

    sleep(15);
    

    gprof_terminate(tid);

    return 0;
}