#include "./../header_files/tasks.h"

void* C3_task(void* param)
{
	fflush(stdout);
	struct timeval waiting_start_tv, waiting_end_tv, turnaround_start_tv, turnaround_end_tv;
	double waiting_time = 0.0, turnaround_time = 0.0;
	thread_args c3 = *(thread_args*)param;
	long long i = 0, sum = 0;
	int n = c3.work_load;

	// get time to calculate WT and TAT
	gettimeofday(&turnaround_start_tv, NULL);
	gettimeofday(&waiting_start_tv, NULL);
	sem_wait(c3.mutex);
	gettimeofday(&waiting_end_tv, NULL);
	waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;

	// I/O operations and computing sum
	FILE* file = fopen(c3.filename, "r");
	while (!feof(file) && n--) {
		fscanf(file, "%lld", &i);
		sum += i;
	}
	fclose(file);
	gettimeofday(&turnaround_end_tv, NULL);
	turnaround_time += (turnaround_end_tv.tv_sec - turnaround_start_tv.tv_sec) + (double)(turnaround_end_tv.tv_usec - turnaround_start_tv.tv_usec)/1000000;

	printf("Total waiting time for C3 = %lf seconds\n", waiting_time);
	printf("Total turnaround time for C3 = %lf seconds\n", turnaround_time);
	file = fopen("C3_FCFS.csv", "a");
	fprintf(file, "%d,%lf,%lf\n", c3.work_load, turnaround_time, waiting_time);
	fclose(file);

	
	open(c3.pfds[WRITE]);
	close(c3.pfds[READ]);
	write(c3.pfds[WRITE], &sum, sizeof(long long int));
	close(c3.pfds[WRITE]);
	pthread_exit(0);
}