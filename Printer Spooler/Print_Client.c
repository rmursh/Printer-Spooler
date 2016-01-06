/*
 * Print_Client.c
 *
 * Created on: Oct 11, 2015
 * Author: Razi Murshed
 * ID: 260516333
 * Description: Simulates the clients that use the printer to print jobs. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

//Defining a fixed size bufffer
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

//Global Variables passed in as parameters from the command line
int ClientId,JobDuration,PagesToPrint; 

FIFO *shared_jobs_ptr; //Pointer to the shared data

//Function to attach the client to the servers memory space
void attach_shared_mem()
{
	const char *name = "/shm-test2";  //Key to be used to map memory space
	int shm_fd; 
	shm_fd = shm_open(name, O_RDWR, 0666); // Opening the file
    //Check for errors 
	if (shm_fd == -1)
	{
			printf("cons: Shared memory failed: %d\n", strerror(errno));
			EXIT_FAILURE;
	}
	// Map the shared memory to the process's address space 
	shared_jobs_ptr = (FIFO*) mmap(0, sizeof(FIFO), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    //Check for errors in mapping
	if (shared_jobs_ptr == MAP_FAILED)
	{
			printf("Map failed\n");
			EXIT_FAILURE;;
	}
}

//Function simulating the client
void client(int clientID, int pagesToPrint, int jobDuration ) {
    //Attach to Shared memory
    attach_shared_mem();
	int value;
	//Testing if the buffer is full to notify the user. 
	int test = sem_getvalue(&shared_jobs_ptr->is_buffer_empty, &value);
	if (value <= 0)
	{
	  printf("Client ID=%u has %u page(s) to print - buffer is currently full - going to sleep.\n", clientID, pagesToPrint);
	}
	// Entering critical section 
	sem_wait(&shared_jobs_ptr->is_buffer_empty);
	sem_wait(&shared_jobs_ptr->mutex);
	// Storing a job in the buffer. 
	shared_jobs_ptr->data_print[shared_jobs_ptr->end] = pagesToPrint;
	shared_jobs_ptr->client_id[shared_jobs_ptr->end] = clientID;
	shared_jobs_ptr->job_duration[shared_jobs_ptr->end] = jobDuration;
	printf("Client ID=%u has %u page(s) to print in %u seconds, the job has been buffered to shared_jobs_ptr[%d].\n", clientID, pagesToPrint, jobDuration, shared_jobs_ptr->end);
	//Incrementing th circular buffer. 
	shared_jobs_ptr->end = (shared_jobs_ptr->end + 1)%BUFFER_SIZE;
	// Exiting critical section 
	sem_post(&shared_jobs_ptr->mutex);
	sem_post(&shared_jobs_ptr->is_buffer_full);
    return;
}


//Main function
int main(int argc, char *argv[]){

	//Checking for correct inputs
	if (argc != 4)
	{
		printf("Need three commands : Cannot Process\n");
		EXIT_FAILURE;
		return -1;
	}
    //Checking if arguments are correct type
	if (sscanf(argv[1], "%i", &ClientId) != 1)
	{
		printf("The Client ID is not an integer number. Try again \n");
		EXIT_FAILURE;
		return -1;
	}
	else if (sscanf(argv[2], "%i", &PagesToPrint) != 1)
	{
		printf("Pages to be printed has to be an integer number. Try again \n");
		EXIT_FAILURE;
		return -1;
	}
	else if (sscanf(argv[3], "%i", &JobDuration) != 1)
	{
		printf("Duration of jobs  has to be an integer number. Try again \n");
		EXIT_FAILURE;
		return -1;
	}
    //Check for empty of negative values from input
	else if (ClientId < 1 || JobDuration < 1 || PagesToPrint < 1)
	{
		printf("Negative or empty character\n");
		EXIT_FAILURE;
		return -1;
	}
	//Run client
	client(ClientId, PagesToPrint, JobDuration);
	EXIT_SUCCESS;
}


