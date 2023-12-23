// C program to demonstrate working of Semaphores 
#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <signal.h>
#include <queue>
#include <time.h>
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KNRM  "\x1B[0m"

#define BUFFER_SIZE 3
#define THREADS_NO  5

int counter=0;
std::queue<int> buffer;
sem_t mutex1,mutex2,full,empty; 


struct arg_struct {    //struct of parameters for the threads.
    int thread_num;       
    
};

void print_queue(std::queue<int> q)
{
   std::queue<int>duplicate=q;
   int c=0;
   while(!duplicate.empty())
   {
   	printf("%s%d\t",KNRM,duplicate.front());
   	duplicate.pop();
   	c++;
   }
   while(c<BUFFER_SIZE)
   {
   	printf("%s%d\t",KNRM,0);
   	c++;
   }
   printf("\n");
}

void* mCounter_thread(void *arguments) 
{ 
	int lower_bound = 1; // Lower bound of the range
	int upper_bound = 5; // Upper bound of the range
	int sleep_time;


	struct arg_struct *args = (struct arg_struct *)arguments;
	//printf("\n%sthread %d is created\n",KNRM,args->thread_num);
        printf("\n%sCounter thread %d: received a message.\n",KBLU,args->thread_num); 
        while (1)
        {
		printf("\n%sCounter thread %d: waiting to write.\n",KBLU,args->thread_num); 
		//wait 
		sem_wait(&mutex1); 
		
		//critical section 
		counter++;
		printf("\n%sCounter thread %d: now adding to counter, counter value=%d.\n",KBLU,args->thread_num,counter); 
		//sleep(4); 
		
		//signal 
		//printf("\n%sThread %d Just Exiting critical section...\n",KRED,args->thread_num); 
		sem_post(&mutex1); 
		srand(time(NULL));
		sleep_time = (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
		sleep(sleep_time);
		printf("\n%sCounter thread %d: received a message.\n",KBLU,args->thread_num); 
	}
	return NULL;
} 




void* mMonitor_thread(void *args) 
{

	int lower_bound = 10; // Lower bound of the range
	int upper_bound = 20; // Upper bound of the range
	int sleep_time;
	int counter_holder;
	int empty_value;

//If the buffer is full: Monitor thread: Buffer full!!
	while (1)
        {
        	printf("\n%sMonitor thread: waiting to read counter.\n",KYEL); 
		//wait 
		sem_wait(&mutex1); 
		
		//critical section
		counter_holder=counter; 
		counter=0;
		printf("\n%sMonitor thread: reading a count value of%d.\n",KYEL,counter_holder); 
		//sleep(4); 
		
		//signal 
		sem_post(&mutex1); 
		//sleep(1);
        
//-------------------------------------------------------------------------------------------------------
		sem_getvalue(&empty,&empty_value);
		if(empty_value==0)
		{
			printf("%sMonitor thread: Buffer full!!",KYEL);
		}
		//wait 
		sem_wait(&empty);
		sem_wait(&mutex2);
		
		//printf("%sPRODUCER: (msg %d) Entered critical section..\n",KYEL,i);
		printf("\n%sMonitor thread: writing to buffer at position %d.\n",KYEL,(int)buffer.size()); 
		//critical section 
		buffer.push(counter_holder);
		//sleep(1);
		 
		sem_post(&mutex2); 
		//signal 
		printf("%s In producer:\narray= ", KNRM);
		
		print_queue(buffer);
		sem_post(&full); 
		//sleep after 
		srand(time(NULL));
		sleep_time = (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
		sleep(sleep_time); 
	}
	    return NULL;
} 

void* mCollector_thread(void *args) 
{
	int lower_bound = 25; // Lower bound of the range
	int upper_bound = 35; // Upper bound of the range
	int sleep_time;
	int full_value,i;

	while (1)
        {
		sem_getvalue(&full,&full_value);
		if(full_value==0)
		{
			printf("%sCollector thread: nothing is in the buffer!.\n",KGRN); 
		}

		//wait 
		sem_wait(&full);
		sem_wait(&mutex2);
		
		//i=buffer.front();
		buffer.pop();
		printf("%sCollector thread: reading from the buffer at position %d.\n",KGRN,0); 
		
		printf("%s In consumer:\narray= ", KNRM);
		
		print_queue(buffer); 

		//sleep(1); 
		sem_post(&mutex2); 
		//signal
		sem_post(&empty); 
		//sleep after 
		srand(time(NULL));
		sleep_time = (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
		sleep(sleep_time); 
	}
	    return NULL;
	
} 

void intHandler(int dummy) {
	// set the noramal color back
    printf("%sExit\n", KNRM);
	// Destroy the semaphore 
	sem_destroy(&mutex1);
	sem_destroy(&mutex2); 
	sem_destroy(&full); 
	sem_destroy(&empty); 
	exit(0);
}

int main() 
{ 
	signal(SIGINT, intHandler);
	
	sem_init(&mutex1, 0, 1); 
	
	pthread_t threads[THREADS_NO] ;
	int i =0;
	for (i=0;i<THREADS_NO;i++)
	{
		arg_struct* args = (struct arg_struct*) malloc(sizeof(struct arg_struct));
                args->thread_num = i;
		pthread_create(&threads[i],NULL,mCounter_thread,(void *)args); 
	}
	
	
	// counter sem set to 1 mutex 
	sem_init(&mutex2, 0, 1); 
	sem_init(&full, 0, 0); 
	sem_init(&empty, 0, BUFFER_SIZE); 
	pthread_t t1,t2;
	pthread_create(&t1,NULL,mCollector_thread,NULL); 
	sleep(2); 
	pthread_create(&t2,NULL,mMonitor_thread,NULL);
	
	for (i=0;i<THREADS_NO;i++){
		pthread_join(threads[i],NULL); 
	}
	 
	pthread_join(t1,NULL); 
	pthread_join(t2,NULL);
	
	sem_destroy(&mutex1);	 
	sem_destroy(&mutex2); 
	sem_destroy(&full); 
	sem_destroy(&empty); 
	printf("%sExit\n", KNRM);
	return 0; 
} 


