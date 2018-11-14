/*
* Program to implement Heap Management
* using malloc352() and free352.
* Author: Simon Haile May 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "heap352.h"
#include "HeapTestEngine.h"
#include "firstfitmodule.h"

#define MINALLOC 64 // minimum request to sbrk()
#define MAXALLOC 1024 //maximum request to sbrk()
#define REGARRAYSIZE 12 // The array of regular size blocks has size 12
typedef double Align[2]; // to force alignment of free-list blocks
						// with the worst-case data boundaries
						//header for free-lists' entry
union header{
	struct{
		union header *next; // pointer to next block in free-list
		unsigned int size; // size, in header units (1 unit is 16 bytes) OF 						//this block
	} data;
	Align alignment; //not used, forces union to occupy 16bytes for bound
};					//boundary alignment

typedef union header Header;
void *mallocquick(int requnits);
void dump_regular_freelist ();
//this is the free-list, initially empty
//static Header *misc_freelist = NULL;

//this is the array of free-lists for regular size requests (2-10)
static Header *regular_freelist[REGARRAYSIZE];

/* Function to initialize the misc_freelist
*
*/
static int init_regfreelist(){

	//set up the regular free list
	//Loop through array and NULL all values.

	for(int i=0; i < REGARRAYSIZE; i++){
		regular_freelist[i] = 0;
	}
	return 1;
}

void *malloc352(int nbytes){

	void * ptr = NULL;
	unsigned int bunits;  // the number of Header-size units required
	// from nbytes, calculate the number of Header block units
	bunits = (nbytes - 1) / sizeof(Header) + 1;
 	if(bunits < 1 || bunits > 9 || (regular_freelist[bunits + 1] == 0) ){
		ptr = mallocff(nbytes);
	}
	else {
		ptr = mallocquick(bunits);
	}
	return ptr;
}

/*The regular_freelist is not null at the specified index
* since this gets called if it is not null.
*/
void *mallocquick(int requnits){
	Header *ptr;
	ptr = regular_freelist[requnits + 1];
	regular_freelist[requnits + 1] = regular_freelist[requnits + 1]->data.next;
  printf("malloc %d units \n", requnits);
  dump_regular_freelist();

	return (void *)(ptr +1);
}

/*  function dump_freelist()
 *  logs the blocks of the free-list
 *  parameters: none
 *  return: none
 */
void dump_regular_freelist () {
int size = 0;
		for(int i =2; i <11; i++){
			size = 0;
			Header *curr = regular_freelist[i];
			if (curr != NULL){
					while (1) {
	        		curr = curr->data.next;
							size++;
	        		if (curr == NULL) break;
	    		}
				}
				printf("The regular_freelist at [%d] has size: %d\n",i, size );
	    	printf("\n");
		}
}

/*find the header to which the freed pointer points
*if the size is between 1 and 9 units add it to the regular_freelist
*/
void free352(void *ptr){
	Header *freed = (Header *)ptr -1;
	if(freed->data.size >=1 && freed->data.size <=9){
		freed->data.next = regular_freelist[(freed->data.size) +1];
		regular_freelist[(freed->data.size) +1] = freed;
		//printf("-------------------------------Dumping Regular free list after free-------------------------------\n");
		printf("free a block of size [%d usable units]\n", freed->data.size );
		dump_regular_freelist();
	}
	else{
		freeff(ptr);
	}

}
int main (int argc, char *argv[]){
	init_freelist();
	init_regfreelist();
	init_heap_test();
	heap_test();
	return 0;
}
