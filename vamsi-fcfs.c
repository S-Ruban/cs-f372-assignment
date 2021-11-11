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
 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[3] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };
 
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
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond[0], &lock);
 
    struct multiple_args_c1 c1;
    c1.pfds[0] = param[0];
    c1.pfds[1] = param[1];
    printf("Enter the value of n1 : ");
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
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
 
void *C2_task(int param[])
{
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond[1], &lock);
    struct multiple_args_c2 c2;
    c2.pfds[0] = param[0];
    c2.pfds[1] = param[1];
    printf("Enter the value of n2 : ");
    scanf("%d",&c2.n2);
    printf("Ente file name: ");
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
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
 
void *C3_task(int param[])
{
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond[2], &lock);
    struct multiple_args_c3 c3;
    c3.pfds[0] = param[0];
    c3.pfds[1] = param[1];
    printf("Enter the value of n3 : ");
    scanf("%d",&c3.n3);
    printf("Enter file name: ");
    scanf("%s",c3.filename);
 
 
    FILE *file = fopen(c3.filename, "r");
    long long int i = 0;
    long long int sum = 0 ;
    while(!feof(file) && c3.n3--)
    {
        fscanf(file, "%lld", &i);
        sum += i ;
    }
    fclose(file);
    dup2(c3.pfds[WRITE], 1);
    close(c3.pfds[READ]);
    open(c3.pfds[WRITE]);
    write(c3.pfds[WRITE],&sum,sizeof(long long int));
    close(c3.pfds[WRITE]);
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
 
int main()
{
    int pfds[3][2];
    int status;
    bool* cc ;
    pthread_t task_tid_c1,task_tid_c2,task_tid_c3;
  pthread_attr_t task_attr_c1,task_attr_c2,task_attr_c3;
    cc = shmat(shmget(ftok("assignment.c", 0x45), 3, 0666 | IPC_CREAT), 0, 0);
    cc[0] = cc[1] = cc[2] = true;
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
                pthread_attr_init(&task_attr_c3);
                pthread_create(&task_tid_c3, &task_attr_c3, C3_task,pfds[2]);
                while(cc[2]);
                pthread_cond_signal(&cond[2]);
                pthread_join(task_tid_c3, NULL);
            }
        }
        else
        {
            pthread_attr_init(&task_attr_c2);
            pthread_create(&task_tid_c2, &task_attr_c2, C2_task,pfds[1]);
            while(cc[1]);
            pthread_cond_signal(&cond[1]);
            pthread_join(task_tid_c2, NULL);
        }
    }
    else
    {
        pthread_attr_init(&task_attr_c1);
        pthread_create(&task_tid_c1, &task_attr_c1, C1_task,pfds[0]);
        while(cc[0]);
        pthread_cond_signal(&cond[0]);
        pthread_join(task_tid_c1,NULL);
    }
}
