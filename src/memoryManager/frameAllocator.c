#include "common/assert.h"
#include "common/debug.h"
#include "common/string.h"

#include "memoryManager/frameAllocator.h"
#include "memoryManager/memoryConstants.h"


#ifdef FRAME_TABLE_ALLOC_TEST
#include "frameAllocatorTest.h"
#endif

/*
A static frameAllocator.  We divide up the physical machine memory at compile time for simplicity
Next version of this should be a dynamic frameAllocator
*/

/*
Implementation details:
We use a bitmap, each bit corresponds to a 4KB chunk of physical memory.
In a 256MB system we have 65,536 chunks, the frameTable takes up 8KB
We split the frame table up into 64MB pieces.
First piece is for the Hypervisor (yes this is deliberatly big)
second,third and fourth are for VM's

We use the variable "domain" to tell the allocator whom we are allocating the memory for.
In the static allocator this just tells us which bit of the table to allocate memory from.
*/

u32int* frameTable;

/* initialise the Hypervisors frame table */
void initialiseFrameTable()
{
  //create new frame allocation table
  newFrameAllocationTable();

  /* mark the frametable itself as in use. */
  u32int* retVal;
  u32int addr = (u32int)frameTable;

#ifdef FRAME_ALLOC_DBG
  printf("Address of frame table (addr): %08x\n", addr);
#endif
  u32int endAddr= addr + FRAME_TABLE_ENTRIES/8;

//Won't work with the subsystem test
#ifndef FRAME_TABLE_ALLOC_TEST
  while((addr < endAddr))
  {
    if(0 == allocFrameAddr(addr))
    {
      //we are seriously screwed if this fails, so fall over quickly
      DIE_NOW(0, "ERROR: Frame Table Initialisation failed... Entering infinite loop");
    }
    //increment in CHUNK_SIZE bits
    addr += FRAME_TABLE_CHUNK_SIZE;
  }
#endif

  /* We have a frame table, now mark all frames the hypervisor and frame table currently occupies as in use */
  addr = HYPERVISOR_START_ADDR;
  endAddr = HYPERVISOR_END_ADDR;

  while((addr < endAddr))
  {
    retVal = allocFrameAddr(addr);

    if(0 == retVal)
    {
      //we are seriously screwed if this fails, so fall over quickly
      DIE_NOW(0, "ERROR: Frame Table Initialisation failed... Entering infinite loop");
    }
    //increment in CHUNK_SIZE bits
    addr += FRAME_TABLE_CHUNK_SIZE;
  }
}

/*
TODO: reimplement this to use a linked list and be able to guarantee contiguous memory space
*/

/**
Allocate a single frame
returns 0 if failed, otherwise the address of the allocated frame
**/
u32int* allocFrame(u8int domain)
{
  if(0 == isDomainValid(domain))
  {
    return 0;
  }

  /* Find a single free frame & mark it in use */
  u32int offset = getFreeFrames(1, domain);

  /* return frame address */
  u32int* addr = offsetToAddr(offset);
#ifdef FRAME_ALLOC_DBG
  if ((u32int)addr)
  {
    printf("Allocated Frame: %08x\n", (u32int)addr);
  }
  else
  {
    printf("No free frames found!\n");
  }
#endif
  return addr;
}

/**
Attempt to allocate multiple contiguous frames
returns 0 if failed, otherwise the address of the first allocated frame
 **/
u32int* allocMultipleFrames(u32int numFrames, u8int domain)
{
  if(0 == isDomainValid(domain))
  {
    return 0;
  }

  /* Find n contiguous frames and mark in use */
  u32int offset = getFreeFrames(numFrames, domain);

  /* return frame address */
  u32int* addr;
  addr = offsetToAddr(offset);

#ifdef FRAME_ALLOC_DBG
  if ((u32int)addr)
  {
    printf("Allocated Frame: %08x\n", (u32int)addr);
  }
  else
  {
    printf("No free frames found!");
  }
#endif
  return addr;
}

/**
* Allocate a single frame given the physical address
* return 0 if frame allocation fails
* else returns the address
**/
u32int* allocFrameAddr(u32int phyAddr)
{
#ifdef FRAME_ALLOC_DBG
  printf("Allocating frame for addr: %08x\n", phyAddr);
#endif
  // get offset into the frame table
  u32int offset = addrToOffset(phyAddr);

  u32int* returnVal = 0;  //assume failure
  if(!isFrameUsed_offset(offset))
  {
    markFrameUsed_offset(offset);
    returnVal = (u32int*) phyAddr;
  }
#ifdef FRAME_ALLOC_DBG
  else
  {
    printf("Frame already allocated.\n");
  }
#endif
  return returnVal;
}

/**
 *  Check if a physical address frame is in use
 **/
u8int isFrameFree(u32int addr)
{
  u32int offset = addrToOffset(addr);
  return isFrameUsed_offset(offset);
}

/**
* Given a physical address, marks the frame as free
* returns 0 on sucess, non zero on failure
**/
u8int freeFrame(u32int phyAddr)
{
  // get offset into the frame table
  u32int offset = addrToOffset(phyAddr);

  return freeFrame_offset(offset);
}

/**
* Given a physical address range, marks all frames as free
* returns 0 on sucess, non zero on failure
**/
u8int freeMultipleFrames(u32int firstFrameAddr, u32int lastFrameAddr)
{
  //check lastFrameAddr is greater than firstFrameAddr
  if(lastFrameAddr < firstFrameAddr)
  {
#ifdef FRAME_ALLOC_DBG
    printf("Failure to free multiple frames, lastFrameAddr : %08x\n", lastFrameAddr);
    printf("Is before first frame addr: %08x\n", firstFrameAddr);
#endif
    return 2;
  }

  u32int numFrames = (lastFrameAddr - firstFrameAddr) / FRAME_TABLE_CHUNK_SIZE;

  // get offset into the frame table
  u32int offset = addrToOffset(firstFrameAddr);
  u32int offsetMax = offset + numFrames;

  while (offset <= offsetMax)
  {
    if(freeFrame_offset(offset))
    {
      //If a free failed, return failure
#ifdef FRAME_ALLOC_DBG
      printf("Failure to free multiple frames, stopped at :%08x\n", (u32int)offsetToAddr(offset));
#endif
      return 1;
    }
    offset++;
  }
  return 0;
}

/* Private Methods */

//returns 0 if not a valid domain, non zero if valid
u8int isDomainValid(u8int domain)
{
  //The static implementation is for 4 domains 0,1,2 & 3
  if((domain < 4))
  {
    return 1;
  }
  else
  {
#ifdef FRAME_ALLOC_DBG
    printf("Error domain: %x, is not valid\n", domain);
#endif
    return 0;
  }
}

//create a new frame allocation table
void newFrameAllocationTable()
{
#ifndef FRAME_TABLE_ALLOC_TEST
  //Ensure frame table chunk size is a power of two
  COMPILE_TIME_ASSERT((0 == (FRAME_TABLE_CHUNK_SIZE & (FRAME_TABLE_CHUNK_SIZE -1)) ), frame_table_chunk_size_not_power_of_two );

  //Ensure frame table is 4KB chunk aligned
  COMPILE_TIME_ASSERT(((FRAME_TABLE_START_ADDRESS & 0x1F) == 0), frame_table_start_addr_invalid);

  frameTable = (u32int*)FRAME_TABLE_START_ADDRESS;
#else
  frameTable = getFrameAllocatorTestAddress();
#endif

#ifdef FRAME_ALLOC_DBG
  printf("Initialise Hypervisor Frame Allocator\n");
  printf("Machine Memory: %x\n", TOTAL_MACHINE_RAM);
  printf("HYPERVISOR_START_ADDR: %x\n", HYPERVISOR_START_ADDR);
  printf("HYPERVISOR_SIZE: %x\n", HYPERVISOR_SIZE);
  printf("FRAME_TABLE_CHUNK_SIZE: %x\n", FRAME_TABLE_CHUNK_SIZE);
  printf("Number of Frame Table entries: %x\n", FRAME_TABLE_ENTRIES);
  printf("Frame table address: %x\n", (u32int)frameTable);
  printf("Frame table size (Bytes): %x\n", FRAME_TABLE_ENTRIES);
#endif

  memset(frameTable, 0, (FRAME_TABLE_ENTRIES/8));

#ifdef FRAME_ALLOC_DBG
  if(frameTable == 0)
  {
    DIE_NOW(0, "memset zeroed bss section. Frame Allocator:newFrameAllocationTable()");
  }
#endif
}


/**
* Take a physical address and return the offset into the frame table
**/
u32int addrToOffset(u32int phyAddr)
{
  //Physical RAM starts at address 0x80000000
  u32int correctedAddress = (phyAddr - MEMORY_START_ADDR);

  //we could get given any address, but we only care about those on CHUNK_SIZE boundaries
  correctedAddress = ((correctedAddress + FRAME_TABLE_CHUNK_SIZE -1) & ~0xFFF);

  /* Could do a right shift by 12, but if FRAME_TABLE_CHUNK_SIZE is changed then the code would break */
  u32int offset = correctedAddress / FRAME_TABLE_CHUNK_SIZE;

  return offset;
}

/**
 * Take a frame table offset and return a physical address
 * returns 0 if out of bounds
 **/
u32int* offsetToAddr(u32int offset)
{
  //bounds check
  if(offset > FRAME_TABLE_ENTRIES)
  {
    return 0;
  }

  COMPILE_TIME_ASSERT(FRAME_TABLE_CHUNK_SIZE == (1 << 12), _fix_offset_to_address_calculation_ );
  u32int address = offset << 12;

  //Physical RAM starts at address 0x80000000
  address += MEMORY_START_ADDR;

  return (u32int*)address;
}

/**
* Takes an offset into the frame table
* returns zero if frame is not in use
* non-zero if frame is in use
**/
u8int isFrameUsed_offset(u32int offset)
{
  //check offset isn't out of bounds
  if(offset > FRAME_TABLE_ENTRIES)
  {
#ifdef FRAME_ALLOC_DBG
    printf("Error: offs into frame table out of bounds. Offs is: %x, but max value is: %x\n",
           offset, FRAME_TABLE_ENTRIES);
#endif
    //say that the frame is inUse
    return 1;
  }

  u8int* frameTableBytewise;
  frameTableBytewise = (u8int*) frameTable;

  //load the byte out of the frame table
  u32int index = (offset & ~0x7) >> 3;
  u8int byte = frameTableBytewise[index];

  //the mask for the bit in the byte we care about
  u8int bit = 1 << (offset & 0x7);

  return (byte & bit);
}

/**
* Takes an offset into the frame table
*
* Returns zero on success, non-zero on failure
**/
u8int markFrameUsed_offset(u32int offset)
{
  //check offset isn't out of bounds
  if(offset > FRAME_TABLE_ENTRIES)
  {
#ifdef FRAME_ALLOC_DBG
    printf("Error: offs into frame table out of bounds. Offs is: %x, but max value is: %x\n",
           offset, FRAME_TABLE_ENTRIES);
#endif

    return 1; //fail
  }

  u8int* frameTableBytewise;
  frameTableBytewise = (u8int*) frameTable;

  //load the byte
  u32int index = (offset & ~0x7) >> 3;
  u8int byte = frameTableBytewise[index];
  u8int bit = 1 << (offset & 0x7);

  //OR in the bit to mark it as in use
  byte = byte | bit;
  frameTableBytewise[index] = byte;

  return 0; //success
}

/**
* Free a frame of memory
* returns 0 on success, non-zero on failure
**/
u8int freeFrame_offset(u32int offset)
{
  //check offset isn't out of bounds
  if(offset > FRAME_TABLE_ENTRIES)
  {
#ifdef FRAME_ALLOC_DBG
    printf("Error: offs into frame table out of bounds. Offs is: %x, but max value is: %x\n",
           offset, FRAME_TABLE_ENTRIES);
#endif

    return 1; //fail
  }

  u8int* frameTableBytewise;
  frameTableBytewise = (u8int*) frameTable;

  //load the byte
  u32int index = (offset & ~0x7) >> 3;
  u8int byte = frameTableBytewise[index];
  u8int bit = 1 << (offset & 0x7);

  //clear the set bit
  byte = byte & ~bit;
  frameTableBytewise[index] = byte;

  return 0; //success
}

//returns the offset into the array for the given domain
u32int getDomainOffset(u8int domain)
{
  //works fine when memmory is statically allocated, this needs cleaning up a bit, for now its a nasty hack
  COMPILE_TIME_ASSERT(((FRAME_TABLE_ENTRIES / 4) == (1 << 14)), _fix_getDomainOffset_);

  //multiply by 16384
  return domain << 14;
}

//returns the highest offset in the array for the given domain
u32int getDomainMaxOffset(u8int domain)
{
  //works fine when memmory is statically allocated, this needs cleaning up a bit, for now its a nasty hack
  COMPILE_TIME_ASSERT(((FRAME_TABLE_ENTRIES / 4) == (1 << 14)), _fix_getDomainMaxOffset_);
  return ((domain + 1) << 14) -1 ;
}

/**
 * Attempts to find a contiguous section of X frames
**/
u32int getFreeFrames(u32int numFrames, u8int domain)
{
  if(numFrames > 32)
  {
    DIE_NOW(0, "Implement getFreeFrames for cases of more than 32 frames. Entering infinite loop");
    /*
    Research suggests maintaining a linked list of free space, containing a start + end offset?
    When we want a frame we search the list and provide the "best" match slot, or chop an existing one.

    on initialisation we would automatically get a single entry containing the whole space.
    everytime we allocate a frame we could cut a bit off.

    The first time space is freed, then we might get a second entry by re-adding it to the free frames list
    */
    return -1;
  }

  /* First we need to get the offset into the frame table for the domain */
  u32int wordIndex = (getDomainOffset(domain) >> 5); //need to divide by 32, to convert the bit offset
  u32int maxWordIndex = (getDomainMaxOffset(domain) >> 5); //need to divide by 32, to convert the bit offset

  if(numFrames > 1)
  {
    bool frameFound = FALSE;
    u32int value;

    while(!frameFound)
    {
      value = frameTable[wordIndex];
      //hacky method to get things running for now, waster memory should be filled in by single frame alloc
      if(value != 0)
      {
        //get next frame
        wordIndex++;
      }
      else
      {
        //allocate it
        //construct a mask
        u32int mask = 0xFFFFFFFF >> (32 - numFrames);

        //write it back to the frameTable
        frameTable[wordIndex] = mask;
        frameFound = TRUE;
        return (wordIndex << 5);
      }
    }
  }
  else
  {
    /* Else do case for single frame */
    do
    {
      u32int value = frameTable[wordIndex];
      if(value != (u32int)-1)
      {
        //we have a found at least one free frame, search for it
        u32int bitIndex = 1, tmp = 0;
        u32int offset = (wordIndex << 5);

        //unrolled binary search
        tmp = value & 0xFFFF;
        if(tmp == 0xFFFF)
        {
          //No free frames, so its in the other halfword
          bitIndex = bitIndex << 16;
          tmp = value >> 16;
          offset +=16;
        }
        value = tmp & 0xFF;
        if(value == 0xFF)
        {
          bitIndex = bitIndex << 8;
          value = tmp >> 8;
          offset +=8;
        }
        tmp = value & 0xF;
        if(tmp == 0xF)
        {
          bitIndex = bitIndex << 4;
          tmp = value >> 4;
          offset +=4;
        }
        value = tmp & 0x3;
        if(value == 0x3)
        {
          bitIndex = bitIndex << 2;
          value = tmp >> 2;
          offset +=2;
        }
        tmp = value & 0x1;
        if(tmp == 0x1)
        {
          //the other bit
          bitIndex = bitIndex << 1;
          offset +=1;
        }
        //found free frame, mark it as used
        frameTable[wordIndex] |= bitIndex;
        return offset;
      }
    }while(++wordIndex <= maxWordIndex);
  }

  //No free frames found
  //Highest value is most likly to be invalid.  By the time we have to worry about 64bit address spaces this will have been rewritten
  DIE_NOW(0, "Frame Allocator: Out of free frames/memory to allocate.");
  return -1;
}
