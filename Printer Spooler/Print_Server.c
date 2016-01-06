

/*
 * Print_Sever.c
 *
 *  Created on: Oct 11, 2015
 *      Author: Razi Murshed
 *      I.D - 260516333
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>

//Defining a fixed sized buffer
#define BUFFER_SIZE 10

//Defining a struct and implementing a circular queue
typedef struct
{
	int start; //Start index of queue
	int end; // End index of queue
	int data_print[BUFFER_SIZE]; //Number of pages to Print
	int client_id[BUFFER_SIZE]; // Id of the client for a particular job
	int job_duration[BUFFER_SIZE]; // Length of a job
	sem_t is_buffer_empty, is_buffer_full, mutex; //Including Semaphores
} FIFO; //Struct to be used as shared data

int C,J,B;


FIFO *shared_jobs_ptr; //Pointer to the shared data

//Function to attach the client to the servers memory space
void setup_shared_mem()
{

	int shm_fd;
	const char *name = "/shm-test2"; //Key to be used to map memory space   
	shm_fd = shm_open(name, O_CREAT|O_RDWR, 0666); // Creating and opening the file
	//Check for errors 
	if (shm_fd == -1)
	{
			printf("cons: Shared memory failed: %d\n", strerror(errno));
			EXIT_FAILURE;
	}
	ftruncate(shm_fd, sizeof(shared_jobs_ptr)); //Adjust size to size of the struct
	// Map the shared memory to the process's address space 
	shared_jobs_ptr = (FIFO*) mmap(0, sizeof(FIFO), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	//Check for errors in mapping
	if (shared_jobs_ptr == MAP_FAILED)
	{
			printf("Map failed\n");
			EXIT_FAILURE;
	}
}

//Initialise semaphores
void init_semaphore(int num)
{   
	//Semaphore for buffer full 
	if(sem_init(&shared_jobs_ptr->is_buffer_full, 1, 0) == -1)
    {
      printf("Error creating semaphore\n");
      EXIT_FAILURE;
	}
	//Semaphore for buffer empty
	if(sem_init(&shared_jobs_ptr->is_buffer_empty, 1, num) == -1)
	{
	  printf("Error creating semaphore\n");
	  EXIT_FAILURE;
	}
	//Mutex
	if(sem_init(&shared_jobs_ptr->mutex, 1, 1) == -1)
	{
	  printf("Error creating semaphore\n");
	  EXIT_FAILURE;
	}
}

//Initialize the buffer
void init_buffer()
{
	shared_jobs_ptr->start=0;
	shared_jobs_ptr->end=0;
}

void terminator(int sig)
{
	if(sig == SIGINT)
	{
		printf("Exiting the program /n");
		munmap(0,sizeof(FIFO)); 
		shm_unlink("/shm-test2");
		EXIT_SUCCESS;
	}
}


//Simulates Printer
void printer(void)
{
	setup_shared_mem();
	init_semaphore(BUFFER_SIZE);
	init_buffer();

	while (1)
	{
		
		signal(SIGINT, terminator);
		int value,test;
		//Check if the buffer is empty and tells the user. 
		test = sem_getvalue(&shared_jobs_ptr->is_buffer_full, &value);  //take_a_job(&job);
		if (value <= 0)
		{
			printf("No request in buffer, Printer sleeps.\n");//print_a_msg(&job);
		}
		// Semaphores controlling the entry in the critical section 
		sem_wait(&shared_jobs_ptr->is_buffer_full);
		sem_wait(&shared_jobs_ptr->mutex);
		//Taking a job from the buffer. 
		unsigned int bufferPos = shared_jobs_ptr->start;
		int currentJob = shared_jobs_ptr->data_print[bufferPos];
		int curentJobDuration = shared_jobs_ptr->job_duration[bufferPos];
		shared_jobs_ptr->start = (shared_jobs_ptr->start+1) % BUFFER_SIZE;
		printf("Printer starts printing %u page(s) from Buffer[%d].\n",  currentJob, bufferPos); //print_a_msg(&job);
		//Exiting the critical section 
		sem_post(&shared_jobs_ptr->mutex);
		sem_post(&shared_jobs_ptr->is_buffer_empty);
		//Sleeping until the job is done. 
		sleep(curentJobDuration);   //go_sleep(&job)
		printf("Printer finishes printing %u page(s) from Buffer[%d].\n",  currentJob, bufferPos); //print_a_msg(&job);
		//shm_unlink("/shm-test");
	}
	return;
}

//main method
int main(int argc, char *argv[])
{
	//Runs Printer
	printer();
	
	EXIT_SUCCESS;
}



