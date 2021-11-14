#include "./../header_files/tasks.h"

void* C3_task(void* param)
{
	thread_args c3 = *(thread_args*)param;
	pthread_mutex_lock(c3.lock);
	pthread_cond_wait(c3.cond, c3.lock);
	printf("Enter the value of work_load 3: ");
	scanf("%d", &c3.work_load);
	c3.filename = "nums.txt";
	struct timeval start_tv, end_tv;
	double total_time;
	gettimeofday(&start_tv, NULL);
	FILE* file = fopen(c3.filename, "r");
	long long i = 0, sum = 0;
	while (!feof(file) && c3.work_load--) {
		fscanf(file, "%lld", &i);
		sum += i;
	}
	fclose(file);
	gettimeofday(&end_tv, NULL);
	total_time = (end_tv.tv_sec - start_tv.tv_sec) + (double)(end_tv.tv_usec - start_tv.tv_usec)/1000000;
	printf("Total turnaround time for C3 = %lf seconds\n", total_time);
	dup2(c3.pfds[WRITE], 1);
	close(c3.pfds[READ]);
	write(c3.pfds[WRITE], &sum, sizeof(long long int));
	close(c3.pfds[WRITE]);
	pthread_mutex_unlock(c3.lock);
	pthread_exit(0);
}