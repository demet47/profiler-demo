#ifndef PROFILER_H
#define PROFILER_H

#include <pthread.h>

void gprof_init(pthread_t tid);

void gprof_terminate(pthread_t tid);

#endif