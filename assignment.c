#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
// gcc -pthread assignment.c -o assignment
char *cc ;
// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER ;
// pthread_cond_t keys[3] = {PTHREAD_COND_INITIALIZER};
struct multiple_args_c1
{
    int n1 ;
    int pfds[2];
};
struct multiple_args_c2
{
    char file[256];
    int pfds[2];
};
void *C1_task(void *param)
{
    // pthread_mutex_lock(&lock);
    // pthread_cond_wait(&keys[0], &lock);
    int n ;
    long long sum = 0 ;
    printf("Enter %d numbers : \n", ((struct multiple_args_c1*)param)->n1);
    for(int i = 0; i < ((struct multiple_args_c1*)param)->n1; i++)
    {
        scanf("%d", &n);
        sum += n ;
    }
    char output[20];
    sprintf(output, "C1 Sum = %lld\n", sum);
    dup2(((struct multiple_args_c1*)param)->pfds[1], 1);
    close(((struct multiple_args_c1*)param)->pfds[0]);
    write(((struct multiple_args_c1*)param)->pfds[1], output, 20);
    // pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
void *C2_task(void *param)
{
    // pthread_mutex_lock(&lock);
    // pthread_cond_wait(&keys[1], &lock);
    FILE *file = fopen(((struct multiple_args_c2*)param)->file, "r");
    char str[1024];
    while(fgets(str, 1024, file) != NULL)
    {
        printf("%s", str);
    }
    fclose(file);
    dup2(((struct multiple_args_c2*)param)->pfds[1], 1);
    close(((struct multiple_args_c2*)param)->pfds[0]);
    write(((struct multiple_args_c2*)param)->pfds[1], "Done Printing\n", 14);
    // pthread_mutex_unlock(&lock);
    pthread_exit(0);
}
void *C3_task(void *param)
{
    // pthread_mutex_lock(&lock);
    // pthread_cond_wait(&keys[2], &lock);
    FILE *file = fopen(((struct multiple_args_c2*)param)->file, "r");
    int i ;
    long long sum = 0 ;
    fscanf(file, "%d", &i);
    while(!feof(file))
    {
        sum += i ;
        fscanf(file, "%d", &i);
    }
    fclose(file);
    char output[20];
    sprintf(output, "C3 Sum = %lld\n", sum);
    dup2(((struct multiple_args_c2*)param)->pfds[1], 1);
    close(((struct multiple_args_c2*)param)->pfds[0]);
    write(((struct multiple_args_c2*)param)->pfds[1], output, 20);
    // pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

int main()
{
    int pfds[2];
    int s ;
    pthread_t monitor_tid, task_tid ;
	pthread_attr_t monitor_attr, task_attr ;
    cc = shmat(shmget(ftok("assignment.c", 0x45), 3, 0666 | IPC_CREAT), 0, 0);
    cc[0] = 0x0 ;
    pipe(pfds);
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
                printf("Enter the order of scheduling the processes : ");
                scanf("%d %d %d", &order[0], &order[1], &order[2]);
                switch(choice)
                {
                    case 1:
                        // FCFS
                        for(int i = 0; i < 3; i++)
                        {
                            cc[0] = (0x1 << (order[i]-1));
                            while(cc[0] == (0x1 << (order[i]-1)));
                            read(pfds[0], buffer, 30);
                            printf("%s\n", buffer);
                        }
                        wait(&s);
                        wait(&s);
                        wait(&s);
                        break ;
                    case 2:
                        // RR
                        
                        break ;
                    default:
                        printf("Enter either 1 or 2.\n\n");
                }
                return 0 ;
            }
            else
            {
                // C3

                while(cc[0] != 0x4);
                struct multiple_args_c2 c3 ;
                strcpy(c3.file, "nums.txt");
                for(int i = 0; i < 2; i++)
                    c3.pfds[i] = pfds[i];
                pthread_attr_init(&task_attr);
                pthread_create(&task_tid, &task_attr, C3_task, (void *)&c3);
                pthread_join(task_tid, NULL);
                cc[0] &= 0xfb ;


            }
        }
        else
        {
            // C2

            while(cc[0] != 0x2);
            struct multiple_args_c2 c2 ;
            strcpy(c2.file, "nums.txt");
            for(int i = 0; i < 2; i++)
                c2.pfds[i] = pfds[i];
            pthread_attr_init(&task_attr);
            pthread_create(&task_tid, &task_attr, C2_task, (void *)&c2);
            pthread_join(task_tid, NULL);
            cc[0] &= 0xfd ;


        }
    }
    else
    {
        // C1

        while(cc[0] != 0x1);
        struct multiple_args_c1 c1 ;
        printf("Enter the value of n1 : ");
        scanf("%d", &c1.n1);
        for(int i = 0; i < 2; i++)
            c1.pfds[i] = pfds[i];
        pthread_attr_init(&task_attr);
        pthread_create(&task_tid, &task_attr, C1_task, (void *)&c1);
        pthread_join(task_tid, NULL);
        cc[0] &= 0xfe ;

    }
}