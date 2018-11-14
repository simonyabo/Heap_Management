/*
    CSCI 352 Assignment 1
    Heap Management functions for first-fit strategy

    David Bover, April 2016
    Simon Haile, May 2017
*/

#include <stdio.h>
#include <unistd.h>
//#include "heap352.h"
#include "HeapTestEngine.h"
#include "firstfitmodule.h"

#define MINALLOC    64      // minimum request to sbrk(), Header blocks

typedef long Align;     // to force alignment of free-list blocks
                        // with worst-case data boundaries

// header for free-list entry
union header {
    struct {
        union header *next; // pointer to next block in free-list
        unsigned int size;  // size, in header units, of this block
    } data;

    Align alignment;        // not used, just forces boundary alignment
};

typedef union header Header;

// this is the free-list, initially empty
static Header *free_list = NULL;

void do_free (void *ptr);


/*  function display_block()
 *  displays one block of the free list
 *  parameter: curr, pointer to the block
 *  return: none
 */
 void display_block (Header *curr) {

    // address of this block
    unsigned long curr_addr = (unsigned long)curr;

    // address of next block in the free-list
    unsigned long next_addr = (unsigned long)curr->data.next;

    // address of next block in the heap, possibly an allocated block
    unsigned long next_mem  = curr_addr + (curr->data.size + 1) * sizeof(Header);

    printf("free block:0x%x, %4d units, next free:0x%x next block:0x%x\n",
           (int)curr_addr, curr->data.size + 1, (int)next_addr, (int)next_mem);
}


/*  function dump_freelist()
 *  logs the blocks of the free-list
 *  parameters: none
 *  return: none
 */
 void dump_freelist () {

    Header *curr = free_list;

    while (1) {
        display_block(curr);
        curr = curr->data.next;
        if (curr == free_list) break;
    }
    printf("\n");
}


/*  function more_heap()
 *  uses sbrk() system call to get more heap space
 *  parameter: nblocks, int, the number of units requested
 *  return: pointer to the new allocated heap-space
 */
 void *more_heap (unsigned nunits) {

    void *mem;
    Header *block;
  //  unsigned long mem_addr;

    // apply minimum request level for sbrk()
    if (nunits < MINALLOC) nunits = MINALLOC;

    // get sbrk() to provide more heap space
    mem = sbrk(nunits * sizeof(Header));
  //  mem_addr = (unsigned long) mem;
  //  printf("(sbrk %d 0x%x) ",
  //         (unsigned)(nunits * sizeof(Header)), (unsigned)mem_addr);
    if (mem == (void *)(-1)) return NULL;

    // set up a free-list block with the new space
    block = (Header *)mem;
    block->data.size = nunits -1;

    // call freeff() to add this block to the free-list
    // initially, add it to the allocated count
    // the call to freeff() will add it to the free-list
    do_free((void *)(block + 1));

    return mem;
}

/*  function coalesce()
 *  joins two adjacent free-list blocks into one larger block
 *  parameter: curr, Header *, address of the first (lower address) block
 *  return: int, 1 if the blocks were joined, 0 otherwise
 */
 int coalesce (Header *curr) {
    Header *pos = curr;
    Header *next_block = pos->data.next;
  //  unsigned long pos_addr = (unsigned long)pos;
  //  unsigned long next_addr = (unsigned long)next_block;

    // try to coalesce with the next block
    if (pos + pos->data.size + 1 == next_block) {
        pos->data.size += next_block->data.size + 1;
        pos->data.next = next_block->data.next;
        return 1;
    }
    // the blocks were not adjacent, so coalesce() failed
    return 0;
}

/* function init_freelist()
 * initializes the free list
 */
  int init_freelist() {

    void *mem;                  // address of heap-space from sbrk() call
    Header *block;

    mem = sbrk(MINALLOC * sizeof(Header));
    if (mem == (void *)(-1)) {
        printf("sbrk() failed\n");
        return -1;
    }

    // set up a size 0 Header for the start of the free_list
    block = (Header *)mem;
    block->data.size = 0;
    block->data.next = block + 1;
    free_list = block;

    // set up the next block, containing the usable memory
    // size is nunits-2 since one block is used for the free-list
    // header and another for the header of this first block
    block = (Header *)(block + 1);
    block->data.size = MINALLOC - 2;
    block->data.next = free_list;

  //dump_freelist();

    return 0;
}

/* function do_malloc()
 * the real heap allocation function
 * allocates (at least) nbytes of space on the heap
 * parameter: nbytes (int), size of space requested
 * return: address of space allocated on the heap
 */

 void *do_malloc (int nbytes) {

    unsigned int nunits;        // the number of Header-size units required
    Header *curr, *prev;        // used in free-list traversal

    // from nbytes, calculate the number of Header block units
    nunits = (nbytes - 1) / sizeof(Header) + 1;

    // first-fit algorithm, free-list blocks are arrange in order of address
    // search through the free-list, looking for sufficient space
    prev = free_list;
    curr = prev->data.next;

    while (curr != free_list)  {

        // exact fit
        if (nunits == curr->data.size)
        {
            prev->data.next = curr->data.next;
            return (void *)(curr + 1);
        }

        // larger space than needed
        if (nunits < curr->data.size) {
            curr->data.size -= (nunits + 1);
            curr += curr->data.size + 1;
            curr->data.size = nunits;
            return (void *)(curr + 1);
        }

        // move along to next block
        prev = curr;
        curr = curr->data.next;
    }

    // sufficient space not found in any free-list block,
    // request more heap space and try the allocation request again
    if (more_heap(nunits))
        return do_malloc(nbytes);
    else
        return NULL;
}

/* function mallocff()
 * a wrapper for do_malloc, with log output
 * allocates (at least) nbytes of space on the heap
 * parameter: nbytes (int), size of space requested
 * return: address of space allocated on the heap
 */
void *mallocff (int nbytes) {

    void *allocated = do_malloc(nbytes);
    return allocated;
}

/*  function do_free()
 *  adds a previously-allocated space to the free-list
 *  parameter: ptr, void*, address of area being freed
 */
 void do_free (void *ptr) {

    Header *block, *curr;
    int coalesced;

    // block points to the header of the freed space
    block = (Header *)ptr - 1;

    // traverse the free-list, place the block in the correct
    // place, to preserve ascending address order
    curr = free_list;
    while (block > curr) {
        if (block < curr->data.next || curr->data.next == free_list) {

            // need to place block between curr and curr->data.next
            block->data.next = curr->data.next;
            curr->data.next = block;

            // attempt to coalesce with the previous block
            coalesced = 1;
            if (curr != free_list)
                // attempt to coalesce with next neighbor
                coalesced = coalesce(curr);

            if (!coalesced || curr == free_list)
                curr = curr->data.next;

            coalesce(curr);
            break;
        }

        // move along the free-list
        curr = curr->data.next;
    }

}

/*  function freeff()
 *  wrapper for a call to do_free()
 *  adds a previously-allocated space to the free-list
 *  parameter: ptr, void*, address of area being freed
 */
void freeff (void *ptr) {

    unsigned long ptr_address = (unsigned long) ptr;
    printf("free 0x%x\n", (unsigned)ptr_address);

    do_free(ptr);
}
