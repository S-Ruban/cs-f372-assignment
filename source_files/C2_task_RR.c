#include "./../header_files/tasks.h"
void* C2_task_RR(void* param)
{
	thread_args c2 = *(thread_args*)param;
	pthread_mutex_lock(c2.lock);
	pthread_cond_wait(c2.cond, c2.lock);
	pthread_mutex_unlock(c2.lock);
	struct timeval start_tv, end_tv;
	double total_time;
	gettimeofday(&start_tv, NULL);
	FILE* file = fopen(c2.filename, "r");
	char str[12];
	while (fgets(str, 12, file) != NULL && c2.work_load--) {
		pthread_mutex_lock(c2.lock);
		pthread_cond_wait(c2.cond, c2.lock);
		printf("%s", str);
		pthread_mutex_unlock(c2.lock);
	}
	fclose(file);
	gettimeofday(&end_tv, NULL);
	total_time = (end_tv.tv_sec - start_tv.tv_sec) + (double)(end_tv.tv_usec - start_tv.tv_usec)/1000000;
	printf("\nTotal turnaround time for C2 = %lf seconds\n", total_time);
	close(c2.pfds[READ]);
	write(c2.pfds[WRITE], "Done Printing", 14);
	close(c2.pfds[WRITE]);
	*c2.shared_memory = 2;
	pthread_exit(0);
}