#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
 
pthread_mutex_t lock =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER,cond2 = PTHREAD_COND_INITIALIZER,cond3 = PTHREAD_COND_INITIALIZER;
 
//https://docs.oracle.com/cd/E19455-01/806-5257/6je9h032r/index.html


int sum = 0;
 
void releasefunc1() 
{ 
    printf("Signaling condition variable cond1\n"); 
	for(int i=0;i<10;i++)
	{
		pthread_cond_signal(&cond1);
		sleep(2);
		printf("IN FOR LOOP %d\n",i);
	}
}
 
void releasefunc2() 
{ 
    printf("Signaling condition variable cond2\n"); 
    pthread_cond_signal(&cond2); 
}
 
void releasefunc3() 
{ 
    printf("Signaling condition variable cond3\n"); 
    pthread_cond_signal(&cond3); 
}
 
void * func1(void *arg)	// prints cumulative sum from 0 to 100
{
	int n = (int)arg;
	pthread_mutex_lock(&lock);
	pthread_cond_wait(&cond1, &lock); 
	for(int i = 0 ; i < n ; i++)
	{	
		pthread_mutex_lock(&lock);
		pthread_cond_wait(&cond1, &lock); 
		sleep(1);//for testing
		sum += i;
		printf("sum = %d\n",sum);
		pthread_mutex_unlock(&lock); 
	}
	pthread_mutex_unlock(&lock); 
	//50 lines,

    // pthread_mutex_lock(&lock);
    // printf("Waiting for second unlock\n");
    // pthread_cond_wait(&cond1, &lock);
    // for(int i=0;i<n;i++)
    // {
    //     sleep(1);
    //     sum-=i;
    //     printf("sum = %d\n",sum);
    // }
    // pthread_mutex_unlock(&lock);
    // printf("Returning thread\n");
	pthread_exit(NULL);
}
 
void * func2(void *arg)	// just prints hello n times
{
	int n = (int)arg;
	pthread_mutex_lock(&lock); 
    pthread_cond_wait(&cond2, &lock); 
	for(int i = 0 ; i < n ; i++)
	{
		sleep(1);
		printf("hello\n");
	}
 
	pthread_mutex_unlock(&lock); 
        printf("Returning thread\n");
	pthread_exit(NULL);
}
 
void * func3(void *arg)	// just prints bye n times
{
	int n = (int)arg;
	pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond3, &lock); 
	for(int i = 0 ; i < n ; i++)
	{
		sleep(1);
		printf("Bye\n");
	}
 
	pthread_mutex_unlock(&lock); 
        printf("Returning thread\n");
	pthread_exit(NULL);
}
 
 
int main()
{
	pthread_t thread1,thread2,thread3;
	int order[3];
	int t = 100;
 
	pthread_create(&thread1, NULL, func1, (void *)t);
	pthread_create(&thread2, NULL, func2, (void *)t);
	pthread_create(&thread3, NULL, func3, (void *)t);
 
        printf("\nGive order arrival, from first to last: ");
        for(int i = 0; i < 3; i++) scanf("%d",&order[i]);
        for(int i = 0; i < 3; i++) printf("%d",order[i]); printf("\n");
 
	for(int i = 0; i < 4; i++)
	{
		switch(order[i%3])
		{
			case 1: { releasefunc1(); pthread_join(thread1, NULL); } break;
			case 2: { releasefunc2(); pthread_join(thread2, NULL); } break;
			case 3: { releasefunc3(); pthread_join(thread3, NULL); } break;
		}
	}
	
	return 0;
}
