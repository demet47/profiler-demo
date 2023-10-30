#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include "kernel.h"
#include <time.h>
#include <errno.h>

#define MAX_PROFILER_NUMBER 5


int counter = 0;


int fdDevCounter = 0;


typdef struct {
    pthread_t tid;
    int kernel_termination_flag = 0;
    sem_t sleepSem;
    sem_t bufferReadDone;
    sem_t bufferFull;
    pthread_t kern_prof_id;

    int* buffer_0;
    int* buffer_1;
    int cursor_0 = 0;
    int cursor_1 = 0;
    int select_buffer = 0;
    int mishap_count = 0;
} KernelMetadata;


KernelMetadata kernel_profiler_instance[MAX_PROFILER_NUMBER];


/*
for multiple case:
carry everything to struct.
when a new open call happens; 
 - we have an instance counter in kernel.c
 - in open function we create the struct. this counter is protected with a mutex so that no simultaneous creations occur
 - with a call to init, we finalize the creation and return the hash id of the struct in struct array. it will be the same as user space
 and will be used in user space
 - in user space i will then create a struct hashed with this id
 - the cpp file will also have struct array hashed with the same value as kernel and user per thread.

 in cpp file to the pid-index hashmap we add pid-counter++ pair

*/

void* kernel_profiler_thread(void* arg){
    pthread_t tid = (pthread_t)arg;
    //with this tid, extract the program stack. symbolic as below:
    struct timespec abs_timeout;
    clock_gettime(CLOCK_REALTIME, &abs_timeout);

    do{
        abs_timeout.tv_sec += 1;

        int result = sem_timedwait(&sleepSem, &abs_timeout);
        if(result == -1 && errno == !ETIMEDOUT) {
            printf("Thread timed out while waiting for the semaphore.\n");
            mishap_count++;
            continue;
        }

        int enough_space = 0;
        if((select_buffer == 0 ? (cursor_0 + 3 <= 10) : (cursor_1 + 3 <= 10))){
            int call_s[3] = {counter++, counter++, counter++};
            enough_space = 1;
            //printf("putting into buffer\n\r");
            if(select_buffer == 0){
                for(int i = 0; i < 3; i++){
                    buffer_0[i+cursor_0++] = call_s[i]; 
                }
            }else{
                for(int i = 0; i < 3; i++){
                    buffer_1[i + cursor_1++] = call_s[i];
                }
            }
        }
        if(!(kernel_termination_flag || enough_space)){
            int semVal = sem_trywait(&bufferReadDone);
            if(semVal != 0){
                printf("copy not done yet\n\r");
                mishap_count++;
            }else{
                select_buffer ^= 1;
                cursor_0 = 0;
                cursor_1 = 0;
                int a = sem_post(&bufferFull);
                if(a == -1){
                    printf("post err\n\r");
                }
                //memset((select_buffer == 0? cursor_0:cursor_1), 0, 10);
            }
        }
    }while(!kernel_termination_flag);
}



//returns 0 for termination flag raise, 1 for not
int psx_devctl(int fdDev, int cmd, void* dataChannel, int size, int* extraData){//extraDAta value passed
    if(cmd == 0){
        pthread_create(&kern_prof_id, NULL, kernel_profiler_thread, extraData);
        printf("created kernel thread\n\r");
    }else if(cmd == 1){
        int a = sem_wait(&bufferFull);
        if(a == -1){
            printf("wait err\n\r");
        }
        memcpy(dataChannel, (select_buffer == 1? buffer_0:buffer_1), 10);
        sem_post(&bufferReadDone);
        if(kernel_termination_flag == 1){
            printf("autonomous termination\n\r");
            return 1;
        }
    }else if(cmd == 2){
        printf("termination initiated\n\r");
        kernel_termination_flag = 1;
        pthread_join(kern_prof_id, NULL);
        printf("kernel thread destroyed\n\r");
    }
    return 0;
}

//returns fDev
int psx_devctl_open(){

    KernelMetadata* instance = (KernelMetadata*) malloc(sizeof(KernelMetadata));
    


    kernel_profiler_instance[fdDevCounter++];
    sem_init(&sleepSem, 0, 0);
    sem_init(&bufferReadDone, 0, 1);
    sem_init(&bufferFull,0,0);
    buffer_1 = (int*) malloc(sizeof(int) * 10);
    buffer_0 = (int*) malloc(sizeof(int) * 10);
    if(buffer_0 == NULL || buffer_1 == NULL){
        printf("MALLOC FAIL\n\r");
    }
    return fdDevCounter - 1;
}


void psx_devctl_close(){
    printf("destroyin psx\n\r");
    if(buffer_0 != NULL){
        printf("ESD\n\r");
        free(buffer_0);
        printf("ESD2\n\r");
    }

    if(buffer_1 != NULL){
        printf("ESD4\n\r");
        free(buffer_1);
        printf("ESD5\n\r");
    }
    
    printf("DINE\n\r");
}
