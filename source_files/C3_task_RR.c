#include "./../header_files/tasks.h"

void* C3_task_RR(void* param)
{
	// printf("Entering C3\n");
	// extern bool time_C3;
	extern struct timeval program_start;
	struct timeval waiting_start_tv, waiting_end_tv, turnaround_start_tv, turnaround_end_tv;
	double waiting_time = 0.0, turnaround_time = 0.0;
	thread_args c3 = *(thread_args*)param;
	long long i = 0, sum = 0;
	int n = c3.work_load;

	gettimeofday(&turnaround_start_tv, NULL);
	// gettimeofday(&waiting_start_tv, NULL);
	// pthread_mutex_lock(c3.lock);
	// pthread_cond_wait(c3.cond, c3.lock);
	// sem_wait(c3.mutex);
	// printf("C3: wait over\n");
	// gettimeofday(&waiting_end_tv, NULL);
	// waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;

	// pthread_mutex_unlock(c3.lock);
	
	FILE* file = fopen(c3.filename, "r");
	while (!feof(file) && n--) {
		gettimeofday(&waiting_start_tv, NULL);
		// pthread_mutex_lock(c3.lock);
		// pthread_cond_wait(c3.cond, c3.lock);
		sem_wait(c3.mutex);
		gettimeofday(&waiting_end_tv, NULL);
		// if (time_C3)
		// {
		// 	time_C3 = false;
		// 	gettimeofday(&C3_start, NULL);
		// 	double C3_current_time = (double)(C3_start.tv_sec - program_start.tv_sec) + (double)(C3_start.tv_usec - program_start.tv_usec)/1000000;
		// 	printf("C3 starts at %lf\n", C3_current_time);
		// }

		waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;
		// printf("In C3\n");
		fscanf(file, "%lld", &i);
		sum += i;
		// pthread_mutex_unlock(c3.lock);
	}
	fclose(file);
	gettimeofday(&turnaround_end_tv, NULL);
	turnaround_time += (turnaround_end_tv.tv_sec - turnaround_start_tv.tv_sec) + (double)(turnaround_end_tv.tv_usec - turnaround_start_tv.tv_usec)/1000000;

	printf("Total waiting time for C3 = %lf seconds\n", waiting_time);
	printf("Total turnaround time for C3 = %lf seconds\n", turnaround_time);
	file = fopen("C3_RR.csv", "a");
	fprintf(file, "%d,%lf,%lf\n", c3.work_load, turnaround_time, waiting_time);
	fclose(file);

	// printf("C3 end\n");

	open(c3.pfds[WRITE]);
	close(c3.pfds[READ]);
	write(c3.pfds[WRITE], &sum, sizeof(long long int));
	close(c3.pfds[WRITE]);
	*c3.shared_memory = 1;
	// printf("Exiting C3\n");
	pthread_exit(0);
}