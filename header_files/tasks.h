#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <wait.h>

#define READ 0
#define WRITE 1

#ifndef INCLUDE_GUARD_functions /* Include guard */
#define INCLUDE_GUARD_functions

extern pthread_mutex_t lock;
extern pthread_cond_t cond[3];

typedef struct thread_args {
	int work_load;
	char* filename;
	int pfds[2];
	int *shared_memory;
	double total_time;
	clock_t start_time, end_time;
	pthread_mutex_t* lock;
	pthread_cond_t* cond;

} thread_args;

void* C1_task_RR(void* param);
void* C2_task_RR(void* param);
void* C3_task_RR(void* param);
void* C1_task(void* param);
void* C2_task(void* param);
void* C3_task(void* param);

#endif