#include "./../header_files/tasks.h"

void* C3_task_RR(void* param)
{
	thread_args c3 = *(thread_args*)param;
	pthread_mutex_lock(c3.lock);
	pthread_cond_wait(c3.cond, c3.lock);
	pthread_mutex_unlock(c3.lock);
	struct timeval start_tv, end_tv;
	double total_time;
	gettimeofday(&start_tv, NULL);
	FILE* file = fopen(c3.filename, "r");
	long long int i = 0;
	long long int sum = 0;
	while (!feof(file) && c3.work_load--) {
		pthread_mutex_lock(c3.lock);
		pthread_cond_wait(c3.cond, c3.lock);
		fscanf(file, "%lld", &i);
		sum += i;
		pthread_mutex_unlock(c3.lock);
	}
	fclose(file);
	gettimeofday(&end_tv, NULL);
	total_time = (end_tv.tv_sec - start_tv.tv_sec) + (double)(end_tv.tv_usec - start_tv.tv_usec)/1000000;
	printf("\nTotal turnaround time for C3 = %lf seconds\n", total_time);
	close(c3.pfds[READ]);
	write(c3.pfds[WRITE], &sum, sizeof(long long int));
	close(c3.pfds[WRITE]);
	*c3.shared_memory = 2;
	pthread_exit(0);
}