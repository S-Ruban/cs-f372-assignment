#include "./../header_files/tasks.h"

void* C1_task_RR(void* param)
{
	struct timeval waiting_start_tv, waiting_end_tv, turnaround_start_tv, turnaround_end_tv;
	double waiting_time = 0.0, turnaround_time = 0.0;
	long long sum = 0, x = 0;
	thread_args c1 = *(thread_args*)param;
	// fflush(stdout);
	srand(time(NULL));

	gettimeofday(&turnaround_start_tv, NULL);
	gettimeofday(&waiting_start_tv, NULL);
	// pthread_mutex_lock(c1.lock);
	// pthread_cond_wait(c1.cond, c1.lock);
	sem_wait(c1.mutex);
	gettimeofday(&waiting_end_tv, NULL);
	waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;
	
	// pthread_mutex_unlock(c1.lock);
	for (int i = 0; i < c1.work_load; i++) {
		gettimeofday(&waiting_start_tv, NULL);
		// pthread_mutex_lock(c1.lock);
		// pthread_cond_wait(c1.cond, c1.lock);
		sem_wait(c1.mutex);
		gettimeofday(&waiting_end_tv, NULL);
		waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;
		// printf("In C1\n");
		sum += rand();
		// pthread_mutex_unlock(c1.lock);
	}
	gettimeofday(&turnaround_end_tv, NULL);
	turnaround_time += (turnaround_end_tv.tv_sec - turnaround_start_tv.tv_sec) + (double)(turnaround_end_tv.tv_usec - turnaround_start_tv.tv_usec)/1000000;

	printf("Total waiting time for C1 = %lf seconds\n", waiting_time);
	printf("Total turnaround time for C1 = %lf seconds\n", turnaround_time);
	FILE *file = fopen("C1_RR.csv", "a");
	fprintf(file, "%d,%lf,%lf\n", c1.work_load, turnaround_time, waiting_time);
	fclose(file);

	printf("C1 end\n");

	open(c1.pfds[WRITE]);
	close(c1.pfds[READ]);
	write(c1.pfds[WRITE], &sum, sizeof(long long int));
	close(c1.pfds[WRITE]);
	// control
	*c1.shared_memory = 1;
	printf("exiting C1\n");
	pthread_exit(0);
}

