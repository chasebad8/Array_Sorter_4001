#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include "shared_items_def.h"

#define NUM_CHILDREN 3
#define KEY 2570
/**
Chase Badalato
101072570

2021-03-05

This program will filter all letter chars to left side of array
and number chars to right side of array.  The array is of size 7
**/
int main(void){

	pid_t pid; //returned process ID from forked child
	void *memory_location; //pointer to shared memory
	struct shared_struct *shared_data; //structure of data that will be in shared memory
	int shmid; //the ID of the shared memory
	int p; //child process couter variable
	int semOne; //the id of a binary semaphore that deals with B[2]
	int semTwo; //the id of a binary semaphore that deals with B[4]
	int debug; //if set to one debug messages will be printed
	char inputArray[7]; //the character array that contains the values to be filtered

	//char inputArray[7] = {'5', 'A', '9', 'M', 'W', '6', 'Z'};
	//char inputArray[7] = {'1', '2', '3', 'C', 'D', 'E', 'F'};

	//check if debug mode should be enabled
	printf("Debug mode: (1 or 0): ");
	scanf(" %d", &debug);

	//Gather the 7 values to use from the user
	printf("Enter 7 characters (hit enter after each character): \n");
	for (int i = 0; i < 7; i ++) {
    	scanf(" %c", &inputArray[i]); 
	}


	//create two semaphores that will be used by the child processes
	semOne = semget((key_t) (KEY)    , 1, 0666 | IPC_CREAT);
	set_semvalue(semOne);
	semTwo = semget((key_t) (KEY + 1), 1, 0666 | IPC_CREAT);
	set_semvalue(semTwo);

	//attempt to allocated some shared memory of size shared_struct
	shmid = shmget((key_t) KEY, sizeof(struct shared_struct), 0666 | IPC_CREAT);
	if (shmid == -1) {
		int error = errno;
		fprintf(stderr,"Failed to allocate shared memory: %d\n", error);
		exit(EXIT_FAILURE);
	}

	//attempt to attach the shared memory to the program
	memory_location = shmat(shmid, (void *) 0, 0);
	if (memory_location == (void *) -1) {
		fprintf(stderr,"Failed to attach to the memory location\n");
		exit(EXIT_FAILURE);
	}
	printf("Memory attached at %X\n", memory_location);

	//copy the inputted array into shared memory
	shared_data = (struct shared_struct *) memory_location;
	memcpy(shared_data->dataArray, inputArray, sizeof(inputArray));

	//fork 3 children
	for (p = 0; p < NUM_CHILDREN; p++){
		pid = fork();

		if (pid == -1){
			perror("fork failed");
			exit(1);
		}
		else if (pid == 0){
			break;
		}
	}

	/*****************************************************
					  	    PARENT
	******************************************************/
	if (pid != 0) {
		//wait until all children complete execution
		int n = NUM_CHILDREN;
		while(n > 0){
			wait(NULL);
			n --;
		}

		printf("\n[PARENT] Original [ %s ]\n", inputArray);
		printf("[PARENT] Filtered [ %s ]\n", shared_data->dataArray);

		//detach from the shared memory
		if (shmdt(memory_location) == -1) {
			fprintf(stderr, "Failed to detach memory from program\n");
			exit(EXIT_FAILURE);
		}		
		//free up the shared memory
		if (shmctl(shmid,  IPC_RMID, 0) == -1) {
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			exit(EXIT_FAILURE);
		}

		//delete the two semaphors
		del_semvalue(semOne);
		del_semvalue(semTwo);

		printf("\n[PARENT] Program Complete.\n");
	}

	/*****************************************************
					  	    CHILD
	******************************************************/
	else {
		//the pointer that points to the values in shared memory
		shared_data = (struct shared_struct *) memory_location;
		
		//Each child process deals with 3 index's swapping values if required.  If no swap is needed
		//then the child process sets its flag.  If a single other process swapped then the flags
		//are reset.  Loop until all 3 processes have finished
		while(!(shared_data->pOneComplete && shared_data->pTwoComplete && shared_data->pTreComplete)) {
			shared_data->pOneComplete, shared_data->pTwoComplete, shared_data->pTreComplete = 0;

			/*****************************************************
						  	    B[0], B[1], B[2]
			******************************************************/
			if(p == 0){
				int swapped = 0;

				if(isNum(shared_data->dataArray[0]) && !isNum(shared_data->dataArray[1])) { //compare values of two neighboring index's
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[0], shared_data->dataArray[1]);
					swapped = swap(0, 1, shared_data); //swap the data
				}
				if (!semaphore_p(semOne)) exit(EXIT_FAILURE); //attempt to get the semaphore for index 2
				if (isNum(shared_data->dataArray[1]) && !isNum(shared_data->dataArray[2])) {
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[1], shared_data->dataArray[2]);
					swapped = swap(1, 2, shared_data);	
				}
				if (!semaphore_v(semOne)) exit(EXIT_FAILURE); //release the semaphore for index 2			

				if(!swapped) {
					if(debug) printf("[CHILD %d] did not swap\n", p);
					shared_data->pOneComplete = 1;
					sleep(0.1); //this is only here for cosmetic purposes ... more explained in my README
				}
			}

			/*****************************************************
						  	    B[2], B[3], B[4]
			******************************************************/
			else if(p == 1) {
				int swapped = 0;

				if (!semaphore_p(semOne)) exit(EXIT_FAILURE);
				if(isNum(shared_data->dataArray[2]) && !isNum(shared_data->dataArray[3])) { 
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[2], shared_data->dataArray[3]);	
					swapped = swap(2, 3, shared_data);
				}
				if (!semaphore_v(semOne)) exit(EXIT_FAILURE);

			    if (!semaphore_p(semTwo)) exit(EXIT_FAILURE);
				if(isNum(shared_data->dataArray[3]) && !isNum(shared_data->dataArray[4])) {
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[3], shared_data->dataArray[4]);	
					swapped = swap(3, 4, shared_data);	
				}
				if (!semaphore_v(semTwo)) exit(EXIT_FAILURE);	

				if(!swapped) {
					if(debug) printf("[CHILD %d] did not swap\n", p);
					shared_data->pTwoComplete = 1;
					sleep(0.1);
				}
			}

			/*****************************************************
						  	    B[4], B[5], B[6]
			******************************************************/
			else if(p == 2) {
				int swapped = 0;

				if (!semaphore_p(semTwo)) exit(EXIT_FAILURE);
				if(isNum(shared_data->dataArray[4]) && !isNum(shared_data->dataArray[5])) {
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[4], shared_data->dataArray[5]);	
					swapped = swap(4, 5, shared_data);
				}
				if (!semaphore_v(semTwo)) exit(EXIT_FAILURE);			
			
				if(isNum(shared_data->dataArray[5]) && !isNum(shared_data->dataArray[6])) {
					if(debug) printf("[CHILD %d] %c and %c will be swapped\n", p, shared_data->dataArray[5], shared_data->dataArray[6]);
					swapped = swap(5, 6, shared_data);				
				}
				if(!swapped) {
					if(debug) printf("[CHILD %d] did not swap\n", p);
					shared_data->pTreComplete = 1;
					sleep(0.1);
				}
			}
		}
		exit(EXIT_SUCCESS);
	}
}

/*
 * Function: isNum
 * --------------------
 * checks to see if the char value is a number.  Since
 * numbers 0 - 9 have the ASCII codes "0" - "9" it is 
 * an easy check
 *
 * int num : the character to compare to
 *
 * return -> 1 if the char is a number 0 otherwise
 */
int isNum(int num) {
	if(num >= '0' && num <= '9'){
		return 1;
	}
	else {
		return 0;
	}
}

/*
 * Function: swap
 * --------------------
 * swaps the values of the first given index with the second given
 * index of the pointer to the array in shared memory.  It assumes
 * that the index's are valid
 *
 *  int indexOne : the first index
 *  int indexTow : the second index
 *  void* memory_location : a pointer to a shared memory structure
 */
int swap(int indexOne, int indexTwo, void * memory_location) {
	struct shared_struct *shared_data = (struct shared_struct *) memory_location;
	char tmpChar = shared_data->dataArray[indexOne];
	shared_data->dataArray[indexOne] = shared_data->dataArray[indexTwo];
	shared_data->dataArray[indexTwo] = tmpChar;
	return 1;
}
