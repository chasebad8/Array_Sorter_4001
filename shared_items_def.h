#ifndef SHARED_ITEMS_DEF_H
#define SHARED_ITEMS_DEF_H
#endif

int set_semvalue(int sem_id);
void del_semvalue(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);
int isNum(int num);
int swap(int indexOne, int indexTwo, void * memory_location);
void debugMessages(char message[],int debug);

struct shared_struct {
	char dataArray[7]; //The data array (only 2 values will be accessed by multiple processes)

	int pOneComplete; //Flag indicating that process one is satisfied
	int pTwoComplete; //Flag indicating that process two is satisfied
	int pTreComplete; //Flag indicating that process three is satisfied
};