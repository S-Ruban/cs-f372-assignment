/*
	2019A7PS0033H - T V Chandra Vamsi 
    2019A3PS0443H - Vishwas Vasuki Gautam 
    2019A7PS0009H - Kaustubh Bhanj 
    2019AAPS0236H - Naren Vilva 
    2019A7PS0097H - Ruban S 
    2019A7PS1200H - Sathvik Bhaskarpandit 
    2019AAPS0345H - Shrikrishna Lolla 
    2019AAPS0317H - Suraj S 
*/

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
#include <semaphore.h>

#define READ 0
#define WRITE 1

#ifndef INCLUDE_GUARD_functions /* Include guard */
#define INCLUDE_GUARD_functions

struct timeval program_start, C1_start, C2_start, C3_start;

typedef struct thread_args {
	int work_load;
	char* filename;
	int pfds[2];
	int *shared_memory;
	double total_time;
	clock_t start_time, end_time;
	sem_t* mutex;
} thread_args;

void* C1_task_RR(void* param);
void* C2_task_RR(void* param);
void* C3_task_RR(void* param);
void* C1_task(void* param);
void* C2_task(void* param);
void* C3_task(void* param);

#endif