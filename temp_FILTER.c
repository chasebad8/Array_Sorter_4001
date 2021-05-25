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

**/
int main(void){

	pid_t pid; //returned process ID from forked child
	void *memory_location; //pointer to shared memory
	struct shared_struct *shared_data; //structure of data that will be in shared memory
	int shmid; //the ID of the shared memory
	int p; //child process couter variable
	int semOne; //the id of a binary semaphore that deals with B[2]
	int semTwo; //the id of a binary semaphore that deals with B[4]
	char inputArray[7] = {'5', 'A', '9', 'M', 'W', '6', 'Z'};

	// printf("Enter 7 characters: \n");
	// for (int i = 0; i < 7; i ++) {
 //    	scanf(" %c", &inputArray[i]); 	
	// }

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

		printf("[PARENT] %s\n", inputArray);
		printf("[PARENT] %s\n", shared_data->dataArray);

		//detach from the shared memory
		if (shmdt(memory_location) == -1) {
			fprintf(stderr, "Failed to detach memory from program\n");
			exit(EXIT_FAILURE);
		}		
		if (shmctl(shmid,  IPC_RMID, 0) == -1) {
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			exit(EXIT_FAILURE);
		}

		del_semvalue(semOne);
		del_semvalue(semTwo);


		printf("Parent and children finished\n");
	}

	/*****************************************************
					  	    CHILD
	******************************************************/
	else {
		//the pointer that points to the values in shared memory
		shared_data = (struct shared_struct *) memory_location;

		/*****************************************************
					  	    B[0], B[1], B[2]
		******************************************************/
		if(p == 0){
			for(int i = 0; i < 100; i ++){
				if(isNum(shared_data->dataArray[0]) && (!isNum(shared_data->dataArray[1]))){
					printf("[CHILD %d] swapping ...\n", p);
					char tmpChar = shared_data->dataArray[0];
					shared_data->dataArray[0] = shared_data->dataArray[1];
					shared_data->dataArray[1] = tmpChar;
				}

				if(isNum(shared_data->dataArray[1]) && !isNum(shared_data->dataArray[2])){
					printf("[CHILD %d] Attempting to enter critical section\n", p);
					if (!semaphore_p(semOne)) exit(EXIT_FAILURE);

					char tmpChar = shared_data->dataArray[1];
					shared_data->dataArray[1] = shared_data->dataArray[2];
					shared_data->dataArray[2] = tmpChar;

					if (!semaphore_v(semOne)) exit(EXIT_FAILURE);
					printf("[CHILD %d] Left critical section\n", p);
				}
			}
		}

		/*****************************************************
					  	    B[2], B[3], B[4]
		******************************************************/
		else if(p == 1) {
			for(int i = 0; i < 100; i ++){
				if(isNum(shared_data->dataArray[2]) && !isNum(shared_data->dataArray[3])){
					printf("[CHILD %d] Attempting to enter critical section\n", p);
					if (!semaphore_p(semOne)) exit(EXIT_FAILURE);

					char tmpChar = shared_data->dataArray[2];
					shared_data->dataArray[2] = shared_data->dataArray[3];
					shared_data->dataArray[3] = tmpChar;

					if (!semaphore_v(semOne)) exit(EXIT_FAILURE);
					printf("[CHILD %d] Left critical section\n", p);
				}

				if(isNum(shared_data->dataArray[3]) && (!isNum(shared_data->dataArray[4]))){
					printf("[CHILD %d] Attempting to enter critical section\n", p);
					if (!semaphore_p(semTwo)) exit(EXIT_FAILURE);

					char tmpChar = shared_data->dataArray[3];
					shared_data->dataArray[3] = shared_data->dataArray[4];
					shared_data->dataArray[4] = tmpChar;

					if (!semaphore_v(semTwo)) exit(EXIT_FAILURE);
					printf("[CHILD %d] Left critical section\n", p);
				}
			}
		}

		/*****************************************************
					  	    B[4], B[5], B[6]
		******************************************************/
		else if(p == 2) {
			for(int i = 0; i < 100; i ++){
				if(isNum(shared_data->dataArray[4]) && !isNum(shared_data->dataArray[5])){
					printf("[CHILD %d] Attempting to enter critical section\n", p);
					if (!semaphore_p(semTwo)) exit(EXIT_FAILURE);

					char tmpChar = shared_data->dataArray[4];
					shared_data->dataArray[4] = shared_data->dataArray[5];
					shared_data->dataArray[5] = tmpChar;

					if (!semaphore_v(semTwo)) exit(EXIT_FAILURE);
					printf("[CHILD %d] Left critical section\n", p);
				}

				if(isNum(shared_data->dataArray[5]) && (!isNum(shared_data->dataArray[6]))){
					printf("[CHILD %d] swapping ...\n", p);
					char tmpChar = shared_data->dataArray[5];
					shared_data->dataArray[5] = shared_data->dataArray[6];
					shared_data->dataArray[6] = tmpChar;
				}
			}
		}
		exit(EXIT_SUCCESS);
	}
}

int isNum(int num) {
	if(num >= '0' && num <= '9'){
		return 1;
	}
	else {
		return 0;
	}
}
