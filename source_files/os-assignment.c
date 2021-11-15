#include "./../header_files/tasks.h"

sem_t mutex[3];
int n1, n2, n3, time_quantum;
struct timeval start;

/*
	Arguments:
	1. int choice
		choice = 1 for FCFS
		choice = 2 for RR

	2. int workload
		workload for C1, C2, C3
*/

int main(int argc, char **argv)
{
	int pfds[3][2];
	int* shared_memory;
	int status, choice;
	pthread_t task_tid_c1, task_tid_c2, task_tid_c3;
	shared_memory = shmat(shmget(ftok("os-assignment.c", 0x45), 9, 0666 | IPC_CREAT), 0, 0);
	/* 
		shared_memory[0..2]: state of the thread
		shared_memory[3..5]:
			0 if thread needs to be run
			1 if thread needs to wait
		shared_memory[6..8]:
			0 if task is not complete
			1 if task is complete
	*/

	/*
		shared_memory[i] = 0: task thread is being run
		shared_memory[i] = 1: task thread should wait
		shared_memory[i] = 2: task thread has finished execution and has written to pipe
		shared_memory[i] = 3: main process has read from pipe and acknowledged the process' completion
	*/
	for (int i = 0; i < 3; i++) 
	{
		shared_memory[i] = 1;
		shared_memory[i + 3] = 1;
		shared_memory[i + 6] = 0;
		pipe(pfds[i]);
	}

	// printf("1. First Come First Serve Scheduling (FCFS)\n2. Round Robin Scheduling (RR)\n\nEnter your choice : ");
	// scanf("%d", &choice);
	// printf("Enter the value of n1 n2 n3: ");
	// scanf("%d %d %d", &n1, &n2, &n3);
	// if (choice == 2) {
	// 	printf("Enter time quantum (in microseconds)\n");
	// 	scanf("%d", &time_quantum);
	// }
	// printf("\n");
	
	gettimeofday(&program_start, NULL);

	choice = atoi(argv[1]);
	n1 = n2 = n3 = atoi(argv[2]);

	if (fork())
	{
		if (fork())
		{
			if (fork())
			{
				// M
				char buffer[30];
				if (choice == 1)
				{
					// FCFS
					for (int i = 0; i < 3; i++)
					{
						shared_memory[i] = 0;
						wait(&status);
						open(pfds[i][READ]);
						close(pfds[i][WRITE]);
						long long int temp = 0;
						if (i == 1)
						{
							read(pfds[i][READ], buffer, 30);
							printf("Reading result from C2 pipe: %s\n", buffer);
						}
						else
						{
							read(pfds[i][READ], &temp, sizeof(long long int));
							printf("Reading result from C%d pipe: %lld\n", i, temp);
						}
						close(pfds[i][READ]);
						shared_memory[i] = 1;
					}
				}
				else if (choice == 2) 
				{
					// RR
					while (shared_memory[3] | shared_memory[4] | shared_memory[5])
					{
						for (int i = 0; i < 3; i++) 
						{
							if (shared_memory[i + 3] == 1)
							{
								shared_memory[i] = 0;
								gettimeofday(&start, NULL);
								double current_time = (double)(start.tv_sec - program_start.tv_sec) + (double)(start.tv_usec - program_start.tv_usec)/1000000;
								printf("C%d starts at %lf\n", i + 1, current_time);
								usleep(time_quantum);
								shared_memory[i] = 1;
							}
							else if (shared_memory[i] != 2 && shared_memory[i + 3] == 0)
							{
								shared_memory[i] = 2;
								wait(&status);
							}
						}
					}
					for (int i = 0; i < 3; i++) {
						open(pfds[i][READ]);
						close(pfds[i][WRITE]);
						long long int temp = 0;
						if (i == 1) 
						{
							read(pfds[i][READ], buffer, 30);
							printf("Result from C2 pipe: %s\n", buffer);
						}
						else 
						{
							read(pfds[i][READ], &temp, sizeof(long long int));
							printf("Result from C%d pipe: %lld\n", i + 1, temp);
						}
						close(pfds[i][READ]);
					}
				}
				else 
				{
					printf("Invalid choice, exiting. (Enter either 1 or 2)\n\n");
				}
				shmdt((void *)shared_memory);
				return 0;
			} 
			else
			{
				thread_args thread_3_args;
				thread_3_args.work_load = n3;
				thread_3_args.filename = "randnums.txt";
				thread_3_args.pfds[READ] = pfds[2][READ];
				thread_3_args.pfds[WRITE] = pfds[2][WRITE];
				thread_3_args.mutex = &mutex[2];
				sem_init(&mutex[2], 0, 0);
				thread_3_args.shared_memory = &shared_memory[8];

				if (choice == 1)
				{
					pthread_create(&task_tid_c3, NULL, C3_task, (void*)&thread_3_args);
					while (shared_memory[2]);
					gettimeofday(&C3_start, NULL);
					double C3_current_time = (double)(C3_start.tv_sec - program_start.tv_sec) + (double)(C3_start.tv_usec - program_start.tv_usec)/1000000;
					printf("C3 starts at %lf\n", C3_current_time);
					sem_post(&mutex[2]);
					pthread_join(task_tid_c3, NULL);
					shared_memory[5] = 0;
				}
				else if (choice == 2)
				{
					pthread_create(&task_tid_c3, NULL, C3_task_RR, (void*)&thread_3_args);
					int temp3 = 0;
					while (shared_memory[8] != 1)
					{
						if (!shared_memory[2])
						{
							// C3 can continue execution after post is executed
							sem_post(&mutex[2]);
						}
						else
						{
							usleep(time_quantum);
						}
					}
					pthread_join(task_tid_c3, NULL);
					shared_memory[5] = 0;
				}
			}
		}
		else
		{
			thread_args thread_2_args;
			thread_2_args.work_load = n2;
			thread_2_args.filename = "randnums.txt";
			thread_2_args.pfds[READ] = pfds[1][READ];
			thread_2_args.pfds[WRITE] = pfds[1][WRITE];
			thread_2_args.mutex = &mutex[1];
			sem_init(&mutex[1], 0, 0);
			thread_2_args.shared_memory = &shared_memory[7];
			if (choice == 1) 
			{
				pthread_create(&task_tid_c2, NULL, C2_task, (void*)&thread_2_args);
				while (shared_memory[1]);
				gettimeofday(&C2_start, NULL);
				double C2_current_time = (double)(C2_start.tv_sec - program_start.tv_sec) + (double)(C2_start.tv_usec - program_start.tv_usec)/1000000;
				printf("C2 starts at %lf\n", C2_current_time);
				sem_post(&mutex[1]);
				pthread_join(task_tid_c2, NULL);
				shared_memory[4] = 0;
			}
			else if (choice == 2) 
			{
				pthread_create(&task_tid_c2, NULL, C2_task_RR, (void*)&thread_2_args);
				int temp2 = 0;
				while (shared_memory[7] != 1) 
				{
					if (!shared_memory[1]) 
					{
						sem_post(&mutex[1]);
					}
					else 
					{
						usleep(time_quantum);
					}
				}
				pthread_join(task_tid_c2, NULL);
				shared_memory[4] = 0;
			}
		}
	}
	else 
	{
		thread_args thread_1_args;
		thread_1_args.work_load = n1;
		thread_1_args.filename = "randnums.txt";
		thread_1_args.pfds[READ] = pfds[0][READ];
		thread_1_args.pfds[WRITE] = pfds[0][WRITE];
		thread_1_args.mutex = &mutex[0];
		sem_init(&mutex[0], 0, 0);
		thread_1_args.shared_memory = &shared_memory[6];

		if (choice == 1)
		{
			pthread_create(&task_tid_c1, NULL, C1_task, (void*)&thread_1_args);
			while (shared_memory[0]);
			gettimeofday(&C1_start, NULL);
			double C1_current_time = (double)(C1_start.tv_sec - program_start.tv_sec) + (double)(C1_start.tv_usec - program_start.tv_usec)/1000000;
			printf("C1 starts at %lf\n", C1_current_time);
			sem_post(&mutex[0]);
			pthread_join(task_tid_c1, NULL);
			shared_memory[3] = 0;
		}
		else if (choice == 2)
		{
			pthread_create(&task_tid_c1, NULL, C1_task_RR, (void*)&thread_1_args);
			int temp1 = 0;
			while (shared_memory[6] != 1)
			{
				if (!shared_memory[0])
				{
					sem_post(&mutex[0]);
				}
				else 
				{
					usleep(time_quantum);
				}
			}
			pthread_join(task_tid_c1, NULL);
			shared_memory[3] = 0;
		}
	}
}