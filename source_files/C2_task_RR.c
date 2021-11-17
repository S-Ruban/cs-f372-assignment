#include "./../header_files/tasks.h"
void* C2_task_RR(void* param)
{
	extern struct timeval program_start;
	struct timeval waiting_start_tv, waiting_end_tv, turnaround_start_tv, turnaround_end_tv;
	double waiting_time = 0.0, turnaround_time = 0.0;
	thread_args c2 = *(thread_args*)param;
	char str[12];
	int n = c2.work_load;

	// get time to calculate WT and TAT
	gettimeofday(&turnaround_start_tv, NULL);
	FILE* file = fopen(c2.filename, "r");

	// I/O operations
	while (fgets(str, 12, file) != NULL && n--) {
		gettimeofday(&waiting_start_tv, NULL);
		sem_wait(c2.mutex);
		gettimeofday(&waiting_end_tv, NULL);
		waiting_time += (waiting_end_tv.tv_sec - waiting_start_tv.tv_sec) + (double)(waiting_end_tv.tv_usec - waiting_start_tv.tv_usec)/1000000;
		printf("%s", str);
	}
	fclose(file);
	gettimeofday(&turnaround_end_tv, NULL);
	turnaround_time += (turnaround_end_tv.tv_sec - turnaround_start_tv.tv_sec) + (double)(turnaround_end_tv.tv_usec - turnaround_start_tv.tv_usec)/1000000;
	
	// print TAT and WT for C2
	printf("Total waiting time for C2 = %lf seconds\n", waiting_time);
	printf("Total turnaround time for C2 = %lf seconds\n", turnaround_time);
	
	// writing workload, TAT and WT into csv file
	file = fopen("C2_RR.csv", "a");
	fprintf(file, "%d,%lf,%lf\n", c2.work_load, turnaround_time, waiting_time);
	fclose(file);

	open(c2.pfds[WRITE]);
	close(c2.pfds[READ]);
	write(c2.pfds[WRITE], "Done Printing", 14); // writing result to pipe
	close(c2.pfds[WRITE]);
	*c2.shared_memory = 1; // mark task as complete
	pthread_exit(0);
}