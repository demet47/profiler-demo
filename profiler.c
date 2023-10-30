#include <stdio.h>
#include <pthread.h>
#include "kernel.h"
#include <stdlib.h>
#include "gprof_utils.h"


#define BUFF_SIZE 10

int* callstack_buffer;
int termination_flag = 0;
pthread_t profiler_tid;



void* profiler_thread(void* tid){
    printf("Created user thread\n\r");
    int fdDev = psx_devctl_open();
    (void) psx_devctl(fdDev, 0 ,NULL, NULL, *(pthread_t*)tid);
    while(termination_flag == 0){
        psx_devctl(fdDev, 1, callstack_buffer, sizeof(int) * BUFF_SIZE, NULL);
        process_data(callstack_buffer);
    }
    printf("QQQ\n\r");
    int sampling_miss;
    psx_devctl(fdDev, 2, &sampling_miss, sizeof(int), NULL);
    printf("RRR\n\r");
    psx_devctl_close();
    /*
    int* address_list;
    int len = get_address_list(address_list);
    char** names_list = (char**)malloc(sizeof(char*)*len);
    for(int i = 0; i < len; i++){
        char a[100];
        names_list[i] = a;
    }

    printf("SSS\n\r");
    //fill here address names list

    map_address_to_name(names_list);
    for(int i = 0; i < len; i++){
        free(names_list[i]);
    }
    free(names_list);
    report_data(sampling_miss);
    */
    printf("exiting user profiler thread\n\r");
}


void gprof_init(pthread_t tid){
    callstack_buffer = (int*) malloc(sizeof(int) * BUFF_SIZE);
    pthread_create(&profiler_tid, NULL, profiler_thread, &tid);
}


void gprof_terminate(pthread_t tid){
    printf("TERMMM\n\r");
    if(termination_flag != 1){
         printf("TERMMM2 \n\r");
        termination_flag = 1;
    }
    printf("TERMMM3 \n\r");
    pthread_join(profiler_tid, NULL);
    printf("TERMMM4 \n\r");
    printf("user thread terminated\n\r");
    free(callstack_buffer);
}


