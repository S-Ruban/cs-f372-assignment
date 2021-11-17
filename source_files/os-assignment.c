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

int main(/*int argc, char **argv*/)
{
	int pfds[3][2];
	int* shared_memory;
	int status, choice;
	pthread_t task_tid_c1, task_tid_c2, task_tid_c3;
	shared_memory = shmat(shmget(ftok("os-assignment.c", 0x45), 9, 0666 | IPC_CREAT), 0, 0); // creating shared memory
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

	printf("1. First Come First Serve Scheduling (FCFS)\n2. Round Robin Scheduling (RR)\n\nEnter your choice : ");
	scanf("%d", &choice);
	if (choice != 1 && choice != 2)
	{
		printf("Invalid choice, exiting. (Enter either 1 or 2)\n\n");
		return 0;
	}
	printf("Enter the value of n1 n2 n3: ");
	scanf("%d %d %d", &n1, &n2, &n3);
	if (choice == 2) {
		printf("Enter time quantum (in microseconds)\n");
		scanf("%d", &time_quantum);
	}
	printf("\n");
	
	gettimeofday(&program_start, NULL);

	// choice = atoi(argv[1]);
	// n1 = n2 = n3 = atoi(argv[2]);

	if (fork())
	{
		if (fork())
		{
			if (fork())
			{
				// Main process
				char buffer[30];
				if (choice == 1)
				{
					// FCFS
					for (int i = 0; i < 3; i++)
					{
						shared_memory[i] = 0; // since shared memory is set to 0, process (i + 1) can start execution
						wait(&status); // wait for the process to finish execution
						open(pfds[i][READ]);
						close(pfds[i][WRITE]);
						long long int temp = 0;
						// printing result from child prcoesses
						if (i == 1)
						{
							read(pfds[i][READ], buffer, 30);
							printf("Reading result from C2 pipe: %s\n", buffer);
						}
						else
						{
							read(pfds[i][READ], &temp, sizeof(long long int));
							printf("Reading result from C%d pipe: %lld\n\n", i, temp);
						}
						close(pfds[i][READ]);
						shared_memory[i] = 1; // process (i + 1) does not need to execute anymore
					}
				}
				else 
				{
					// RR
					// while all processes are not finished, keep executing while loop
					while (shared_memory[3] | shared_memory[4] | shared_memory[5])
					{
						for (int i = 0; i < 3; i++) // switching between the 3 processes
						{
							if (shared_memory[i + 3] == 1) // check if current process has not finished
							{
								shared_memory[i] = 0; // current process can execute now
								
								// print time since start of program
								gettimeofday(&start, NULL);
								double current_time = (double)(start.tv_sec - program_start.tv_sec) + (double)(start.tv_usec - program_start.tv_usec)/1000000;
								printf("C%d starts at %lf\n", i + 1, current_time);
								
								// sleep for time_quantum after time_quantum is over
								usleep(time_quantum);
								shared_memory[i] = 1;
							}
							else if (shared_memory[i] != 2 && shared_memory[i + 3] == 0) // process (i + 1) comlpeted task
							{
								shared_memory[i] = 2; // set process (i + 1) to be completed
								wait(&status); // wait for process to end
							}
						}
					}
					// print results
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
				shmdt((void *)shared_memory);
				return 0;
			} 
			else
			{
				// C3 process, initializing attributes
				thread_args thread_3_args;
				thread_3_args.work_load = n3;
				thread_3_args.filename = "randnums.txt";
				thread_3_args.pfds[READ] = pfds[2][READ];
				thread_3_args.pfds[WRITE] = pfds[2][WRITE];
				thread_3_args.mutex = &mutex[2];
				sem_init(&mutex[2], 0, 0);
				thread_3_args.shared_memory = &shared_memory[8];

				if (choice == 1) // C3 FCFS
				{
					pthread_create(&task_tid_c3, NULL, C3_task, (void*)&thread_3_args);

					// monitor thread
					while (shared_memory[2]); // monitor thread waits until C3's turn comes
					
					// get time since program start when C3 starts execution
					gettimeofday(&C3_start, NULL);
					double C3_current_time = (double)(C3_start.tv_sec - program_start.tv_sec) + (double)(C3_start.tv_usec - program_start.tv_usec)/1000000;
					printf("C3 starts at %lf\n", C3_current_time);
					
					sem_post(&mutex[2]); // C3's turn now, signalling semaphore
					pthread_join(task_tid_c3, NULL); // wait for task thread to finish execution
				}
				else if (choice == 2) // C3 RR
				{
					pthread_create(&task_tid_c3, NULL, C3_task_RR, (void*)&thread_3_args);

					// monitor thread
					while (shared_memory[8] != 1) // C3 not completed
					{
						if (!shared_memory[2]) // C3's turn to start/continue execution
						{
							// signal C3 semaphore
							sem_post(&mutex[2]);
						}
						else
						{
							// C3 sleeps until it's turn
							usleep(time_quantum);
						}
					}
					pthread_join(task_tid_c3, NULL); // wait for task thread to finish execution
					shared_memory[5] = 0;
				}
			}
		}
		else
		{
			// C2 process, initializing attributes
			thread_args thread_2_args;
			thread_2_args.work_load = n2;
			thread_2_args.filename = "randnums.txt";
			thread_2_args.pfds[READ] = pfds[1][READ];
			thread_2_args.pfds[WRITE] = pfds[1][WRITE];
			thread_2_args.mutex = &mutex[1];
			sem_init(&mutex[1], 0, 0);
			thread_2_args.shared_memory = &shared_memory[7];

			if (choice == 1) // C2 FCFS
			{
				pthread_create(&task_tid_c2, NULL, C2_task, (void*)&thread_2_args);

				// monitor thread
				while (shared_memory[1]); // monitor thread waits until C2's turn comes
				
				// get time since program start when C2 starts execution
				gettimeofday(&C2_start, NULL);
				double C2_current_time = (double)(C2_start.tv_sec - program_start.tv_sec) + (double)(C2_start.tv_usec - program_start.tv_usec)/1000000;
				printf("C2 starts at %lf\n", C2_current_time);
				
				sem_post(&mutex[1]); // C2's turn now, signalling semaphore
				pthread_join(task_tid_c2, NULL); // wait for task thread to finish execution
			}
			else if (choice == 2) // C2 RR
			{
				pthread_create(&task_tid_c2, NULL, C2_task_RR, (void*)&thread_2_args);

				// monitor thread
				while (shared_memory[7] != 1) // C2 not completed
				{
					if (!shared_memory[1]) // C2's turn to start/continue execution
					{
						// signal C2 semaphore
						sem_post(&mutex[1]);
					}
					else 
					{
						// C2 sleeps until it it's turn
						usleep(time_quantum);
					}
				}
				pthread_join(task_tid_c2, NULL); // wait for task thread to finish execution
				shared_memory[4] = 0;
			}
		}
	}
	else 
	{
		// C1 process, initializing attributes
		thread_args thread_1_args;
		thread_1_args.work_load = n1;
		thread_1_args.filename = "randnums.txt";
		thread_1_args.pfds[READ] = pfds[0][READ];
		thread_1_args.pfds[WRITE] = pfds[0][WRITE];
		thread_1_args.mutex = &mutex[0];
		sem_init(&mutex[0], 0, 0);
		thread_1_args.shared_memory = &shared_memory[6];

		if (choice == 1) // C1 FCFS
		{
			pthread_create(&task_tid_c1, NULL, C1_task, (void*)&thread_1_args);

			// monitor thread
			while (shared_memory[0]);
			
			// get time since program start when C1 starts execution
			gettimeofday(&C1_start, NULL);
			double C1_current_time = (double)(C1_start.tv_sec - program_start.tv_sec) + (double)(C1_start.tv_usec - program_start.tv_usec)/1000000;
			printf("C1 starts at %lf\n", C1_current_time);
			
			sem_post(&mutex[0]); // C1's turn now, signalling semaphore
			pthread_join(task_tid_c1, NULL); // wait for task thread to finish execution
		}
		else if (choice == 2) // C1 RR
		{
			pthread_create(&task_tid_c1, NULL, C1_task_RR, (void*)&thread_1_args);

			// monitor thread
			while (shared_memory[6] != 1)
			{
				if (!shared_memory[0])
				{
					// signal C1 semaphore
					sem_post(&mutex[0]);
				}
				else 
				{
					// C1 sleeps until it's turn
					usleep(time_quantum);
				}
			}
			pthread_join(task_tid_c1, NULL); // wait for task thread to finish execution
			shared_memory[3] = 0;
		}
	}
}