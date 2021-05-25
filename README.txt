SYSC 4001A Assignment 2

Chase Badalato 
101072570

*************
Compiling
*************
1) untar files
2) in the directory run "make" to compile the source code
3) type "./Array_Sorter" to run
4) If you wish to run extra tests manually they are commented out at the top of the FILTER_101072570.
   you would also have to comment out the scanf for loop for the 7 array values. (I don't think this 
   should ever be needed)

******
FILES
******
There should be 6 files total
-Makefile
-FILTER_101072570.c
-semun.h
-shared_items_def.h
-semaphore.c
-README.txt

semun.h and semaphore.c are semaphore implementations that I borrowed from the textbook from lab 4.  They
implement binary sempahores.
shared_items_def.h defines the other functions I use inside of the main along with the structure that is 
shared in memory

*****************
IMPORTANT GOALS
*****************
I knew that this program could be taken in many different directions to arrive at the same solution.  I wanted
to create a program that minimized uneccessary looping after the actual array was sorted which I found to be the
most difficult part.

I knew that having one semaphore for the entire shared memory structure of shared memory array was not going to 
be very efficient.  I began by thinking about how the the array gets divided between all of the processes and 
realized that there is only 2 index's that actually need to be mutually exclusive.  It then made sense to me to
use two seperate sempahores.  Once semaphore for each "critical section" index.  This allowed for non critial
values to be swapped around as required.  I breifly considered that this could still have some mutual exclusion 
issues ... what if the critial index 2 gets swapped with 1, and 1 and 0 get swapped at the same time.  I then realized 
that both of those swappings occur in the same process, which of course runs sequentially so that can never happen.
I think that the way I coded it allows for the fastest and safest implementation.  

I have 3 flags in shared memory along with the array (pOneComplete, pTwoComplete, pTreComplete.  These flags get set 
when the respective child process doesn't need to do any swapping.  Each process will then check if all flags are set
to 1.  If they are not that means at least 1 unaccounted for swap occured and all flags must be checked again.

It should be noted that in my code I have sleep(0.1).  This code is not required at all it is just there so
the console won't be spammed by the processes that aren't currently swapping.  If process 1 is swapping and process 2 
is waiting for the semaphore but process 3 is currently done swapping, p3 of course will loop, always setting its flag
to complete but seeing that the other two flags aren't set and thus looping again.  

If a third state was introduced into the flag status, maybe "not set", the other processes that are done could do some 
form of spin locking until all processes return a real status flag value, but I think that it is uncessecary.

I implemented the semaphores using the code done in lab 4

*************
Algorithm
*************

Each child process working on the array follows the algorith below

while (all 3 processes sorted flags are NOT set) {

	if dealing with critical index's, aquire respective semaphore
	if (index i IS a num AND index i + 1 IS NOT a num)
		swap these indexes
	release the semaphore

	else (no swapping)
		set the processes sorted flag, indicating that 
		it didn't need to do any sorting	
}

*************
TEST RESULTS
*************
I tried running multiple tests against this system and always got the proper sorted array

Some tests I ran that I thought may cause issues were:

"1,2,3,4,5,6,7" (WORKED)
"a,b,c,d,e,f,g" (WORKED)
"1,a,s,d,f,g,h" (WORKED)
"1,2,3,4,5,6,A" (WORKED)
"5,A,9,M,W,6,Z" (WORKED)
"1,2,3,C,D,E,F" (WORKED)

All of these tests passed with no issues.  I tested running with both debugging and no debugging
and also always got the same properly filtered results.  I tried using both the scanned user input
and hardcoded values and both yielded the correct expected results.
