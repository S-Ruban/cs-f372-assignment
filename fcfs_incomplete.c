#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#define READ 0
#define WRITE 1
//Assume the order of process creation is C1, C2 and C3. 
//C1, C2 and C3 are to be scheduled only after all the 3 processes have been created by M
//the emulation of scheduling is to be done using the sleep() function.
// M will use shared memory segments with C1, C2 and C3 to communicate if C1, C2, C3 should sleep or wake up to perform their designated tasks. 
//Within each of C1, C2 and C3, use one thread to do the task (mentioned earlier) and another to monitor communications from M and accordingly put the task thread to sleep, or wake it up
//when M tells a process (either C1 or C2 or C3) to sleep, only the task thread should sleep
//The monitor thread shouldremain awake in order to monitor communications from M. You can decide the frequency at the whichthe monitor thread checks for communication from M or can design some form of notification
//mechanism
//io queue??



//dump into a header file



struct multiple_args_c1
{
    int n1 ;
    int pfds[2];
};
struct multiple_args_c2
{
    char filename[256];
    int pfds[2];
    int n2;
};
struct multiple_args_c3
{
    int n3;
    char filename[256];
    int pfds[2];
};


void *C1_task(int param[])
{
    struct multiple_args_c1 c1;
    c1.pfds[0] = param[0];
    c1.pfds[1] = param[1];
    printf("Enter the value of n1 : ");//need to put into a waiting state so that task thread C2 can be processed
    scanf("%d",&c1.n1);
    long long int sum = 0;
    long long int x = 0;
    for(int i = 0; i < c1.n1; i++)
    {
        srand(time(NULL));
        x = rand();
        sum+=x;
    }
    dup2(c1.pfds[WRITE], 1);
    close(c1.pfds[READ]);
    open(c1.pfds[WRITE]);
    write(c1.pfds[WRITE], &sum, sizeof(long long int));
    close(c1.pfds[WRITE]);
    pthread_exit(0);
}
void *C2_task(int param[])
{
    struct multiple_args_c2 c2;
    c2.pfds[0] = param[0];
    c2.pfds[1] = param[1];
    printf("Enter the value of n2 : ");//need to put into waiting state
    scanf("%d",&c2.n2);
    printf("Ente file name: ");//need to put into waiting state
    scanf("%s",c2.filename);


    FILE *file = fopen(c2.filename, "r");
    char str[12];
    while(fgets(str, 12, file) != NULL && c2.n2--)
        printf("%s", str);
    fclose(file);
    dup2(c2.pfds[WRITE], 1);
    close(c2.pfds[READ]);
    open(c2.pfds[WRITE]);
    write(c2.pfds[WRITE],"Done Printing\n",14);
    close(c2.pfds[WRITE]);
    pthread_exit(0);
}
void *C3_task(int param[])
{
    struct multiple_args_c3 c3;
    c3.pfds[0] = param[0];
    c3.pfds[1] = param[1];
    printf("Enter the value of n3 : ");//need to put into waiting
    scanf("%d",&c3.n3);
    printf("Ente file name: ");//need to put into waiting state
    scanf("%s",c3.filename);


    FILE *file = fopen(c3.filename, "r");
    long long int i = 0;
    long long int sum = 0 ;
    while(!feof(file) && c3.n3--)
    {
        fscanf(file, "%lld", &i);//shift between waiting and running?? dump values into an array and sum would make it one clear waiting and running state
        sum += i ;
    }
    fclose(file);
    dup2(c3.pfds[WRITE], 1);
    close(c3.pfds[READ]);
    open(c3.pfds[WRITE]);
    write(c3.pfds[WRITE],&sum,sizeof(long long int));
    close(c3.pfds[WRITE]);
    pthread_exit(0);
}
 
int main()
{
    int pfds[3][2];//need to make 3 pipes as per rules
    int status;
    bool* cc ;
    pthread_t task_tid_c1,task_tid_c2,task_tid_c3;
	pthread_attr_t task_attr_c1,task_attr_c2,task_attr_c3;
    cc = shmat(shmget(ftok("assignment.c", 0x45), 3, 0666 | IPC_CREAT), 0, 0);
    cc[0] = 1;
    cc[1] = 1;
    cc[2] = 1;
    pipe(pfds[0]);
    pipe(pfds[1]);
    pipe(pfds[2]);
    if(fork())
    {
        if(fork())
        {
            if(fork())
            {
                // M
                int choice ;
                int order[3];
                char buffer[30];
                printf("1. First Come First Serve Scheduling (FCFS)\n2. Round Robin Scheduling (RR)\n\nEnter your choice : ");
                scanf("%d", &choice);
                switch(choice)
                {
                    case 1:
                        // FCFS
                        for(int i = 0; i < 3; i++)
                        {
                            cc[i] = false;
                            wait(&status);
                            open(pfds[i][READ]);
                            close(pfds[i][WRITE]);
                            long long int temp = 0;
                            if(i==1)
                            {
                                read(pfds[i][READ], buffer, 30);
                                printf("M: %s\n", buffer);
                            }
                            else
                            {
                                read(pfds[i][READ],&temp,sizeof(long long int));
                                printf("M: %lld\n", temp);
                            }
                            close(pfds[i][READ]);
                            cc[i] = true;
                        }
                        break;
                    case 2:
                        // RR
                        
                        break;
                    default:
                        printf("Enter either 1 or 2.\n\n");
                }
                return 0 ;
            }
            else
            {
                while(cc[2]);//issue here is 1. only one thread currently exists, 2. sleep is better than while; sleep puts into wait state but while uses cpu (running state)
                pthread_attr_init(&task_attr_c3);
                pthread_create(&task_tid_c3, &task_attr_c3, C3_task,pfds[2]);
                pthread_join(task_tid_c3, NULL);
            }
        }
        else
        {
            while(cc[1]);//issue here is 1. only one thread currently exists, 2. sleep is better than while; sleep puts into wait state but while uses cpu (running state)
            pthread_attr_init(&task_attr_c2);
            pthread_create(&task_tid_c2, &task_attr_c2, C2_task,pfds[1]);
            pthread_join(task_tid_c2, NULL);
        }
    }
    else
    {
        //make this modular?? similar structure for all processes
        while(cc[0]);
        pthread_attr_init(&task_attr_c1);//issue here is 1. only one thread currently exists, 2. sleep is better than while; sleep puts into wait state but while uses cpu (running state)
        pthread_create(&task_tid_c1, &task_attr_c1, C1_task,pfds[0]);
        pthread_join(task_tid_c1,NULL);
    }
}