#include "./../header_files/tasks.h"

void* C1_task_RR(void* param)
{
	fflush(stdout);
	thread_args c1 = *(thread_args*)param;
	pthread_mutex_lock(c1.lock);
	pthread_cond_wait(c1.cond, c1.lock);
	pthread_mutex_unlock(c1.lock);
	struct timeval start_tv, end_tv;
	double total_time;
	gettimeofday(&start_tv, NULL);
	long long sum = 0, x = 0;
	srand(time(NULL));
	for (int i = 0; i < c1.work_load; i++) {
		pthread_mutex_lock(c1.lock);
		pthread_cond_wait(c1.cond, c1.lock);
		sum += rand();
		pthread_mutex_unlock(c1.lock);
	}
	gettimeofday(&end_tv, NULL);
	total_time = (end_tv.tv_sec - start_tv.tv_sec) + (double)(end_tv.tv_usec - start_tv.tv_usec)/1000000;
	printf("\nTotal turnaround time for C1 = %lf seconds\n", total_time);
	close(c1.pfds[READ]);
	write(c1.pfds[WRITE], &sum, sizeof(long long int));
	close(c1.pfds[WRITE]);
	*c1.shared_memory = 2;
	pthread_exit(0);
}

