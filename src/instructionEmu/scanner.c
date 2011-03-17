#include "scanner.h"
#include "blockCache.h"
#include "common.h"
#include "mmu.h"
#include "pageTable.h"
#include "debug.h"

//Uncomment to enable debugging: #define SCANNER_DEBUG
//Uncomment to enable debugging: #define PC_DEBUG


void scanBlock(GCONTXT * gc, u32int blkStartAddr)
{
#ifdef SCANNER_COUNTER
  scannerReqCounter++;
#endif
#ifdef DUMP_SCANNER_COUNTER
  if ((scannerReqCounter % 4000) == 3999)
  {
    serial_putint_nozeros(scannerReqCounter);
    serial_putstring(" ");
    if ((scannerReqCounter % 80000) == 79999)
    {
      serial_newline();
    }
  }
#endif

  struct instruction32bit * decodedInstruction = 0;
  u32int * currAddress = (u32int*)blkStartAddr;
  u32int instruction = *currAddress;
  u32int * blockCopyCacheCurrAddress = ((u32int* )(gc->blockCopyCacheLastUsedLine))+1;
  u32int * blockCopyCacheStartAddress = ((u32int* )(gc->blockCopyCacheLastUsedLine))+1;
  bool reservedWord = 0;//See struct blockCacheEntry in blockCache.h for explanation
  u32int blockCopyCacheSize=0;

  u32int hashVal = getHash(blkStartAddr);
  u32int bcIndex = (hashVal & (BLOCK_CACHE_SIZE-1)); // 0x1FF mask for 512 entry cache

  bool inBlockCache = checkBlockCache(blkStartAddr, bcIndex, gc->blockCache);
  if (inBlockCache)
  {
    //Check the logbook
    BCENTRY * bcEntry = getBlockCacheEntry(bcIndex, gc->blockCache);
    gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
    gc->endOfBlockInstr = bcEntry->hyperedInstruction;

    u32int * addressInBlockCopyCache = 0;
    //The programcounter of the code that is executing should be set to the code in the blockCache
    if(bcEntry->reservedWord)
    {
      addressInBlockCopyCache= (u32int*)bcEntry->blockCopyCacheAddress+2;//First word is a backpointer & 2nd word is reservedWord
    }
    else
    {
      addressInBlockCopyCache= (u32int*)bcEntry->blockCopyCacheAddress+1;//First word is a backpointer
    }

    gc->R15 = (u32int)addressInBlockCopyCache;
    //But also the PC of the last instruction of the block should be set
    gc->PCOfLastInstruction = (u32int)bcEntry->endAddress;

#ifdef SCANNER_DEBUG
    serial_putstring("scanner: Block @ ");
    serial_putint(blkStartAddr);
    serial_putstring(" hash value ");
    serial_putint(hashVal);
    serial_putstring(" cache index ");
    serial_putint(bcIndex);
    serial_putstring(" HIT");
    serial_newline();
#endif
    return;
  }

  //NOT in Block Cache
#ifdef SCANNER_DEBUG
  serial_putstring("scanner: Block @ ");
  serial_putint(blkStartAddr);
  serial_putstring(" hash value ");
  serial_putint(hashVal);
  serial_putstring(" cache index ");
  serial_putint(bcIndex);
  serial_putstring(" MISS!!!");
  serial_newline();
#endif

  //Check if there is room on blockCopyCacheCurrAddres and if not make it
  blockCopyCacheCurrAddress=checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
  //Install Backpointer in BlockCache:
  *(blockCopyCacheCurrAddress++)=(u32int)(gc->blockCache + bcIndex);//pointer arithmetic gc->blcokCache+bcIndex  and save pointer as u32int
  //After the Backpointer the instructions will be installed.  Here the guestprocess should continue it's execution.
  gc->R15=(u32int)blockCopyCacheCurrAddress;

#ifdef SCANNER_DEBUG
  serial_putstring("Backpointer installed at: ");
  serial_putint((u32int)(blockCopyCacheCurrAddress-1));
  serial_putstring("Contents= ");
  serial_putint(*(blockCopyCacheCurrAddress-1));
  serial_newline();
#endif

  while (1)//Just keep on scanning untill function scanBlock returns.
  {
    //binary & checks types -> do a cast of function pointer to u32int
    if((decodedInstruction = decodeInstr(instruction))->replaceCode == 1)
    {  //Critical instruction

        /*----------------Install HdlFunct----------------*/
        //Non of the source registers is the ProgramCounter -> Just End Of Block
        //Finish block by installing SVC
        //Save end of block instruction and handler function pointer close to us...
        gc->endOfBlockInstr = instruction;
        gc->hdlFunct = decodedInstruction->hdlFunct;
        gc->PCOfLastInstruction = (u32int)currAddress;
        // replace end of block instruction with hypercall of the appropriate code
        //Check if there is room on blockCopyCacheCurrAddress and if not make it
        blockCopyCacheCurrAddress = checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
        *(blockCopyCacheCurrAddress++)=(INSTR_SWI | bcIndex);

        // if guest instruction stream is mapped with caching enabled, must maintain
        // i and d cache coherency
        // iCacheFlushByMVA((u32int)currAddress);
        #ifdef SCANNER_DEBUG
          serial_putstring("scanner: EOB @ ");
          serial_putint((u32int)currAddress);
          serial_putstring(" instr ");
          serial_putint(gc->endOfBlockInstr);
          serial_putstring(" SWIcode ");
          serial_putint(bcIndex);
          serial_putstring(" hdlrFuncPtr ");
          serial_putint((u32int)gc->hdlFunct);
          serial_newline();
        #endif

        // asm volatile ("BKPT 0");
        // asm volatile ("BKPT 0");
        //We have to determine the size of the BlockCopyCache
        if(blockCopyCacheCurrAddress<blockCopyCacheStartAddress)
        {
          blockCopyCacheSize+=gc->blockCopyCacheEnd - (u32int)blockCopyCacheStartAddress;
          blockCopyCacheSize+=(u32int)blockCopyCacheCurrAddress - gc->blockCopyCache;
          blockCopyCacheSize=blockCopyCacheSize>>2;//we have casted pointers to u32int thus divide by 4 to get size in words
          #ifdef SCANNER_DEBUG
            serial_putstring("Block exceeding end: blockCopyCacheSize=");
            serial_putint(blockCopyCacheSize);
          #endif
        }
        else
        {
          blockCopyCacheSize=blockCopyCacheCurrAddress-blockCopyCacheStartAddress;
        }

        // add the block we just scanned to block cache
        if(reservedWord)
        {
          addToBlockCache(blkStartAddr&1, (u32int)currAddress,
                                            bcIndex, blockCopyCacheSize, (u32int)blockCopyCacheStartAddress,gc->endOfBlockInstr,(u32int)gc->hdlFunct,gc->blockCache);
        }
        else
        {
          addToBlockCache(blkStartAddr, (u32int)currAddress,
                                                      bcIndex, blockCopyCacheSize, (u32int)blockCopyCacheStartAddress,gc->endOfBlockInstr,(u32int)gc->hdlFunct,gc->blockCache);
        }



        protectScannedBlock(blkStartAddr, (u32int)currAddress);

        //update blockCopyCacheLastUsedLine (blockCopyCacheLastUsedLine is u32int -> add nrOfInstructions*4
        gc->blockCopyCacheLastUsedLine=gc->blockCopyCacheLastUsedLine+((blockCopyCacheCurrAddress-blockCopyCacheStartAddress)<<2);

        #ifdef SCANNER_DEBUG
          serial_putstring("Block added with size of ");
          serial_putint(((u32int)blockCopyCacheCurrAddress-(u32int)blockCopyCacheStartAddress));
          serial_putstring(" words.");
          serial_newline();
        #endif
        /*----------------END Install HdlFunct----------------*/
        return;
    }else
    {//Non critical instruction
      if(allSrcRegNonPC(instruction))
      { //Non of the source registers is the ProgramCounter -> Safe instruction
        //Check if there is room on blockCopyCacheCurrAddress and if not make it
        blockCopyCacheCurrAddress=checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
        //copy instruction to Block Copy Cache
        *(blockCopyCacheCurrAddress++)=instruction;//Copy instruction and update pointer
      }
      else
      {  //One of the source registers of the instruction is the ProgramCounter
         //Perform PCFunct-> necessary information = currAddress,
        /*----------------Execute PCFunct----------------*/
        gc->endOfBlockInstr = instruction;//Not really the endOfBlockInstr but we can use it

        blockCopyCacheCurrAddress= decodedInstruction->PCFunct(gc,currAddress,blockCopyCacheCurrAddress,blockCopyCacheStartAddress);
        if(((u32int)blockCopyCacheCurrAddress & 0b1) == 0b1)
        {//Last bit of returnAddress is used to indicate that a reserved word is necessary -> we can assume 2 byte allignement (even in worst case scenario (thumb))
          reservedWord=1;//ReservedWord is true
          blockCopyCacheCurrAddress=(u32int*)((u32int)blockCopyCacheCurrAddress & 0xFFFFFFFE);//Set last bit back to zero
        }
        /*----------------END Execute PCFunct----------------*/
      }
    }
    // Instruction handled -> go to next instruction
    currAddress++;
    instruction = *currAddress;
  } // decoding while ends
}//end of scanBlock



void protectScannedBlock(u32int startAddress, u32int endAddress)
{
  // 1. get page table entry for this address
  descriptor* ptBase = mmuGetPt0();
  descriptor* ptEntryAddr = get1stLevelPtDescriptorAddr(ptBase, startAddress);

  switch(ptEntryAddr->type)
  {
    case SECTION:
    {
      if ((startAddress & 0xFFF00000) != (endAddress & 0xFFF00000))
      {
        DIE_NOW(0, "protectScannedBlock: Basic block crosses section boundary!");
      }
      addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
      break;
    }
    case PAGE_TABLE:
    {
      u32int ptEntryLvl2 = *(u32int*)(get2ndLevelPtDescriptor((pageTableDescriptor*)ptEntryAddr, endAddress));
      switch(ptEntryLvl2 & 0x3)
      {
        case LARGE_PAGE:
          serial_putstring("Page size: 64KB (large), 0x");
          serial_putint(ptEntryLvl2);
          serial_newline();
          DIE_NOW(0, "Unimplemented.");
          break;
        case SMALL_PAGE:
          if ((ptEntryLvl2 & 0x30) != 0x20)
          {
            addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
          }
          break;
        case FAULT:
          serial_putstring("Page invalid, 0x");
          serial_putint(ptEntryLvl2);
          serial_newline();
          DIE_NOW(0, "Unimplemented.");
          break;
        default:
          DIE_NOW(0, "Unrecognized second level entry");
          serial_newline();
          break;
      }
      break;
    }
    case FAULT:
      serial_putstring("Entry for basic block: invalid, 0x");
      serial_putint(*(u32int*)ptEntryAddr);
      serial_newline();
      DIE_NOW(0, "Unimplemented.");
      break;
    case RESERVED:
      DIE_NOW(0, "Entry for basic block: reserved. Error.");
      break;
    default:
      DIE_NOW(0, "Unrecognized second level entry. Error.");
  }
}

#ifdef DUMP_SCANNER_COUNTER
void resetScannerCounter()
{
  scannerReqCounter = 0;
}
#endif

/* allSrcRegNonPC will return true if all source registers of an instruction are zero  */
u32int allSrcRegNonPC(u32int instruction)
{
  //Source registers correspond with the bits [0..3],[8..11] or [16..19]
  if((instruction & 0xF0000)==0xF0000 || (instruction & 0xF00)==0xF00 || (instruction & 0xF)==0xF )
  {
#ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" has a PC register");
    serial_newline();
#endif
    return 0;//false
  }
  else
  {
#ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" doesn't have a PC register");
    serial_newline();
#endif
    return 1;//true
  }
}


