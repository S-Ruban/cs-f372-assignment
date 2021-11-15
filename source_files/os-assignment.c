#include "./../header_files/tasks.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[3] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };
int n1, n2, n3, time_quantum = 20000;

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
	int status, choice;
	int* shared_memory;
	pthread_t task_tid_c1, task_tid_c2, task_tid_c3;
	shared_memory = shmat(shmget(ftok("os-assignment.c", 0x45), 3, 0666 | IPC_CREAT), 0, 0);
	/*
		shared_memory[i] = 0: task thread is being run
		shared_memory[i] = 1: task thread should wait
		shared_memory[i] = 2: task thread has finished execution and has written to pipe
		shared_memory[i] = 3: main process has read from pipe and acknowledged the process' completion
	*/
	for (int i = 0; i < 3; i++) {
		shared_memory[i] = 1;
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
	
	choice = atoi(argv[1]);
	n1 = n2 = n3 = atoi(argv[2]);

	if (fork()) {
		if (fork()) {
			if (fork()) {
				// M
				char buffer[30];
				if (choice == 1) {
					// FCFS
					for (int i = 0; i < 3; i++) {
						shared_memory[i] = 0;
						wait(&status);
						open(pfds[i][READ]);
						close(pfds[i][WRITE]);
						long long int temp = 0;
						if (i == 1) {
							read(pfds[i][READ], buffer, 30);
							// printf("M: %s\n", buffer);
						} else {
							read(pfds[i][READ], &temp, sizeof(long long int));
							// printf("M: %lld\n", temp);
						}
						close(pfds[i][READ]);
						shared_memory[i] = 1;
					}
				}
				else if (choice == 2) {
					// RR
					
					int completed[3] = { 0, 0, 0 };
					while (!(completed[0] & completed[1] & completed[2])) {
						for (int i = 0; i < 3; i++) {
							if (!completed[i]) {
								shared_memory[i] = 0;
								usleep(time_quantum); // scheduling needs to be done only when time_quantum is over
								// bonus -> if the process finished earlier than time_quantum, then it should be removed immeditaely right??
								// printf("Sleep done\n");
								if (shared_memory[i] == 0) {
									shared_memory[i] = 1;
									// printf("Ended %d\n",i);
									continue;
								} else if (shared_memory[i] == 2) {
									completed[i] = 1;
									shared_memory[i] = 3;
									wait(&status);
									open(pfds[i][READ]);
									close(pfds[i][WRITE]);
									long long int temp = 0;
									if (i == 1) {
										read(pfds[i][READ], buffer, 30);
										// printf("M: %s\n\n", buffer);
									} else {
										read(pfds[i][READ], &temp, sizeof(long long int));
										// printf("M: %lld\n", temp);
									}
									close(pfds[i][READ]);
								}
							}
						}
					}
				}
				else {
					printf("Invalid choice, exiting. (Enter either 1 or 2)\n\n");
				}
				fflush(stdout);
				shmdt((void *)shared_memory);
				return 0;
			} else {
				thread_args thread_3_args;
				thread_3_args.work_load = n3;
				thread_3_args.filename = "randnums.txt";
				thread_3_args.pfds[READ] = pfds[2][READ];
				thread_3_args.pfds[WRITE] = pfds[2][WRITE];
				thread_3_args.lock = &lock;
				thread_3_args.cond = &cond[2];
				thread_3_args.shared_memory = &shared_memory[2];
				if (choice == 1) {
					pthread_create(&task_tid_c3, NULL, C3_task, (void*)&thread_3_args);
					while (shared_memory[2]);
					usleep(50000);
					pthread_cond_signal(&cond[2]);
					pthread_join(task_tid_c3, NULL);
				} else if (choice == 2) {
					pthread_create(&task_tid_c3, NULL, C3_task_RR, (void*)&thread_3_args);

					while (shared_memory[2] != 2) {
						if (!shared_memory[2]) {
							pthread_cond_signal(&cond[2]);
						} else
							usleep(time_quantum);
					}
					pthread_join(task_tid_c3, NULL);
				}
			}
		} else {
			thread_args thread_2_args;
			thread_2_args.work_load = n2;
			thread_2_args.filename = "randnums.txt";
			thread_2_args.pfds[READ] = pfds[1][READ];
			thread_2_args.pfds[WRITE] = pfds[1][WRITE];
			thread_2_args.lock = &lock;
			thread_2_args.cond = &cond[1];
			thread_2_args.shared_memory = &shared_memory[1];
			if (choice == 1) {
				pthread_create(&task_tid_c2, NULL, C2_task, (void*)&thread_2_args);
				while (shared_memory[1]);
				usleep(50000);
				pthread_cond_signal(&cond[1]);
				pthread_join(task_tid_c2, NULL);
			} else if (choice == 2) {
				pthread_create(&task_tid_c2, NULL, C2_task_RR, (void*)&thread_2_args);

				while (shared_memory[1] != 2) {
					if (!shared_memory[1]) {
						pthread_cond_signal(&cond[1]);
					} else
						usleep(time_quantum);
				}
				pthread_join(task_tid_c2, NULL);
			}
		}
	} else {
		thread_args thread_1_args;
		thread_1_args.work_load = n1;
		thread_1_args.filename = "randnums.txt";
		thread_1_args.pfds[READ] = pfds[0][READ];
		thread_1_args.pfds[WRITE] = pfds[0][WRITE];
		thread_1_args.lock = &lock;
		thread_1_args.cond = &cond[0];
		thread_1_args.shared_memory = &shared_memory[0];
		if (choice == 1) {
			pthread_create(&task_tid_c1, NULL, C1_task, (void*)&thread_1_args);
			while (shared_memory[0]);
			usleep(50000);
			pthread_cond_signal(&cond[0]);
			pthread_join(task_tid_c1, NULL);
		} else if (choice == 2) {
			pthread_create(&task_tid_c1, NULL, C1_task_RR, (void*)&thread_1_args);
			// printf("%lf\n",x);
			while (shared_memory[0] != 2) {
				if (!shared_memory[0]) {
					// printf("Signalling\n");
					pthread_cond_signal(&cond[0]);
				} else {
					// printf("Here\n");
					usleep(time_quantum);
				}
			}
			pthread_join(task_tid_c1, NULL);
		}
	}
}