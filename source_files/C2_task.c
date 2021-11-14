#include "./../header_files/tasks.h"

void* C2_task(void* param)
{
	thread_args c2 = *(thread_args*)param;
	pthread_mutex_lock(c2.lock);
	pthread_cond_wait(c2.cond, c2.lock);
	printf("Enter the value of work_load 2: ");
	scanf("%d", &c2.work_load);
	c2.filename = "nums.txt";
	struct timeval start_tv, end_tv;
	double total_time;
	gettimeofday(&start_tv, NULL);
	FILE* file = fopen(c2.filename, "r");
	char str[20];
	while (fgets(str, 20, file) != NULL && c2.work_load--) {
		printf("%s", str);
	}
	fclose(file);
	gettimeofday(&end_tv, NULL);
	total_time = (end_tv.tv_sec - start_tv.tv_sec) + (double)(end_tv.tv_usec - start_tv.tv_usec)/1000000;
	printf("Total turnaround time for C2 = %lf seconds\n", total_time);
	dup2(c2.pfds[WRITE], 1);
	close(c2.pfds[READ]);
	write(c2.pfds[WRITE], "Done Printing\n", 14);
	close(c2.pfds[WRITE]);
	pthread_mutex_unlock(c2.lock);
	pthread_exit(0);
}