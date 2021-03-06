#include "./../header_files/tasks.h"

void* C1_task(void* param)
{
	struct timeval waiting_start_tv, waiting_end_tv, turnaround_start_tv, turnaround_end_tv;
	double waiting_time = 0.0, turnaround_time = 0.0;
	thread_args c1 = *(thread_args*)param;
	long long sum = 0;
	srand(time(NULL));
	
	// get time to calculate WT and TAT
	gettimeofday(&turnaround_start_tv, NULL);
	gettimeofday(&waiting_start_tv, NULL);
	sem_wait(c1.mutex);
	gettimeofday(&waiting_end_tv, NULL);
	waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;

	// calculating sum
	for (int i = 0; i < c1.work_load; i++) {
		sum += rand();
	}
	gettimeofday(&turnaround_end_tv, NULL);
	turnaround_time += (turnaround_end_tv.tv_sec - turnaround_start_tv.tv_sec) + (double)(turnaround_end_tv.tv_usec - turnaround_start_tv.tv_usec)/1000000;

	// print TAT and WT for C1
	printf("Total waiting time for C1 = %lf seconds\n", waiting_time);
	printf("Total turnaround time for C1 = %lf seconds\n", turnaround_time);
	
	// writing workload, TAT and WT into csv file
	/*
	FILE *file = fopen("C1_FCFS.csv", "a");
	fprintf(file, "%d,%lf,%lf\n", c1.work_load, turnaround_time, waiting_time);
	fclose(file);
	*/
	
	open(c1.pfds[WRITE]);
	close(c1.pfds[READ]);
	write(c1.pfds[WRITE], &sum, sizeof(long long int)); // writing result to pipe
	close(c1.pfds[WRITE]);
	pthread_exit(0);
}