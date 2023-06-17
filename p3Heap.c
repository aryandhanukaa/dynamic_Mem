///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2022 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission Fall 2022, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// Main File:        p3Heap.c
//// This File:        p3Heap.c
//// Other Files:      
//// Semester:         CS 354 Fall 2022
//// Instructor:       deppeler
////
//// Author:           Aryan Anupkumar Dhanuka
//// Email:            adhanuka@wisc.edu
//// CS Login:         dhanuka
////
///////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
////                   Fully acknowledge and credit all sources of help,
////                   including Peer Mentors, Instructors, and TAs.
////
//// Persons:          Identify persons by name, relationship to you, and email.
////                   Describe in detail the the ideas and help they provided.
////
//// Online sources:   Avoid web searches to solve your problems, but if you do
////                   search, be sure to include Web URLs and description of
////                   of any information you find.
//////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"
 
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 */

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 * - If a BEST-FIT block found is NOT found, return NULL
 *   Return NULL unable to find and allocate block for desired size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {
	//TODO
  //check validity of heap size
  if(size<=0)
    {
      return NULL;
    }
  if(size>alloc_size)
    {
      return NULL;
    }
  //calculate required size
  int reqS=size+sizeof(blockHeader)+7;
  reqS=reqS-(reqS%8);//adds padding to the calculated required size
  blockHeader* temp=NULL;//stores the block being considered for the allocation
  blockHeader* currB=heap_start;//stores the block being visited
  while(currB->size_status!=1&&(void*)currB<((void*)heap_start+alloc_size))
    {
      int sizeB=(currB->size_status/8)*8;//size of the visited block
      int status=(currB->size_status%8)%2;//whether or not the block is free
	if(status==0)//block is free
	  {
	    if(sizeB==reqS)//size of currBlock is equal to req size
	      { 
	       temp=currB;
		break;
	      }
	    if(sizeB>reqS)//the current block has enough space
	      {
		if(temp!=NULL)
		  {
		    if((temp->size_status/8)*8>sizeB)//curr Block is a better choice for allocation
		      {
			temp=currB;
		      }
		  }
		else{
		  temp =currB;
		}
      
	      }
	  }
      //increment currB
      currB=(blockHeader*)((void*)currB+sizeB);
    }
  if(temp==NULL)//incase no open spots were found
    {
      return NULL;
    }
  if(((temp->size_status/8)*8)!=reqS)//we need to split the block
    {
      int secondblk=(temp->size_status/8)*8-reqS;//this is the size of the second block
      temp->size_status=reqS+1+(temp->size_status%8);//mod 8 retains p bit
      void* tem=(void*) temp;
      blockHeader* blk2=(blockHeader*)(tem+reqS);
      blk2->size_status=secondblk+2;//p bit requires 2 to be added
      ((blockHeader*)((void*)blk2+secondblk-sizeof(blockHeader)))->size_status=secondblk;//creates footer for the newly created block
      
      
    }
  else
    {

	blockHeader* nextB=((blockHeader*)((void*)temp+reqS));
	if(nextB->size_status!=1)
	{
     		nextB->size_status+=2;//updates p bit
	}
      temp->size_status++;//updates a bit
    }

  return (void*)(temp+1);//returns a pointer to the payload
    return NULL;
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int bfree(void *ptr) {    
    //TODO: Your code goes in here.
  //check validity of ptr
  if(ptr==NULL)
    return -1;
  int i=(unsigned int)(ptr);
  if(i%8!=0)
    {
      return -1;
    }
  //ptr is not in the heap
  if((ptr<(void*)heap_start)||(ptr>((void*)heap_start)+alloc_size))
    {
      return -1;
    }
  blockHeader* head= (blockHeader*)((void*)ptr-sizeof(blockHeader));//head of ptr
  int sizeA=(head->size_status/8)*8;//size of ptr
  if((head->size_status%8)%2==0)//ptr has been freed
    {
      return -1;
    }
  //ptr is valid at this point
  ( head->size_status)--;//change state 
  ((blockHeader*)((void*)head+sizeA-sizeof(blockHeader)))->size_status=sizeA;//creates footer
  if(((blockHeader*)((void*)head+sizeA))->size_status!=1)
    {
      ((blockHeader*)((void*)head+sizeA))->size_status-=2;//changes the previous block bit for the next block
    }
  return 0;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for delayed coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
	//TODO
  int cnt=0;//no of coalescings
  blockHeader* temp=heap_start;
  while(temp->size_status!=1)
    {
      int tempSize=(temp->size_status/8)*8;
      if((temp->size_status%8)%2==0)//check a bit=0
	{
      //check p bit=0
	  if(temp->size_status%8==0)
	    {
	      cnt++;
	      
	      int sizeA=tempSize;//size of current block
	      int sizeB=(temp-1)->size_status;//size of prev block
	      ((blockHeader* )((void*)(temp)+sizeA-sizeof(blockHeader)))->size_status=sizeA+sizeB;//footer of curr block
	      ((blockHeader*)((void*)(temp)-sizeB))->size_status+=sizeA;//header of prev block
      
     
	    }
	  
	}
      temp=(blockHeader*)((void*)temp+tempSize);
    }
  return cnt;
       
}

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int pagesize;   // page size
    int padsize;    // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader* end_mark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
       fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
} 
                  
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     
 
    int counter;
    char status[6];
    char p_status[6];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** Block List **********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end of myHeap.c (Spring 2022)                                         


