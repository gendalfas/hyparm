#include "pageTable.h"
#include "memoryConstants.h"
#include "serial.h"
#include "frameAllocator.h"
#include "memFunctions.h" //for memset
#include "mmu.h" // for setDomain
#include "guestContext.h"
#include "beGPTimer.h" /// should not be included here
#include "debug.h"

//Uncomment to enable all page table debugging: #define PT_DEBUG
//Uncomment to show entries being mapped: #define PT_SHORT_DEBUG
//Uncomment to show shadow page table code:#define PT_SHADOW_DEBUG

//Useful to have pageTableDumps with shadow debug
#ifdef PT_SHADOW_DEBUG
#define PT_DUMP_DEBUG
#endif

//Enabled everything
#ifdef PT_DEBUG
#define PT_SMALL_DEBUG
#define PT_LARGE_DEBUG
#define PT_SECTION_DEBUG
#define PT_SUPERSECTION_DEBUG
#define PT_SHADOW_DEBUG
#define PT_SHORT_DEBUG
#define PT_DUMP_DEBUG
#endif


extern GCONTXT * getGuestContext(void);

descriptor* hypervisorPtd;

u32int guestSecondLvlPageTables[256];

/* Don't want to make the hypervisorPtd global */
void setGuestPhysicalPt(GCONTXT* gc)
{
  gc->PT_physical = hypervisorPtd;
}

descriptor* createHypervisorPageTable()
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();
  // zero metadata array
  int i = 0;
  for (i = 0; i < 256; i++)
  {
    guestSecondLvlPageTables[i] = 0;
  }

  //alloc some space for our 1st Level page table
  hypervisorPtd = createNew1stLevelPageTable(HYPERVISOR_FA_DOMAIN);

  //map in the hypervisor
  mapHypervisorMemory(hypervisorPtd);

  //set the domain (access control) for the hypervisor pages
  setDomain(HYPERVISOR_ACCESS_DOMAIN, client);

  //1:1 Map the entire of physical memory

  //small page map the first MB of mem so we can best protect the bootstrap code from self modification
  smallMapMemory(hypervisorPtd, MEMORY_START_ADDR, (MEMORY_START_ADDR + LARGE_PAGE_SIZE -1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  smallMapMemory(hypervisorPtd, (MEMORY_START_ADDR + LARGE_PAGE_SIZE), (MEMORY_START_ADDR +SECTION_SIZE -1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  sectionMapMemory(hypervisorPtd, (MEMORY_START_ADDR +SECTION_SIZE), (HYPERVISOR_START_ADDR-1), GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);

  setDomain(GUEST_ACCESS_DOMAIN, client);

  //serial
  smallMapMemory(hypervisorPtd, UART3, (UART3 + UART3_SIZE -1),
                               GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);


  // uart1
  smallMapMemory(hypervisorPtd, UART1, (UART1 + UART1_SIZE -1), 
                               GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  // clock manager
  smallMapMemory(hypervisorPtd, CLOCK_MANAGER, (CLOCK_MANAGER+CLOCK_MANAGER_SIZE-1),
                               GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0b000);
  // interrupt controller
  smallMapMemory(hypervisorPtd, INTERRUPT_CONTROLLER, (INTERRUPT_CONTROLLER+INTERRUPT_CONTROLLER_SIZE-1),
                          HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer1 - this looks dirty
  smallMapMemory(hypervisorPtd, GPTIMER1, (GPTIMER1+GPTIMER1_SIZE-1),
                          HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer2
  smallMapMemory(hypervisorPtd, GPTIMER2, (GPTIMER2+GPTIMER2_SIZE-1),
                          HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);


  /*
  Exception vectors
  For some reason these do not seem to work unless mapped in as sections?!
  */

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  addSectionPtEntry(hypervisorPtd, exceptionHandlerAddr,exceptionHandlerAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //add section mapping for 0x40200000 (redirected exception vectors)
  //We will want to use the exception handler remap feature to put the page tables in the 0xffff0000 address space later
  const u32int exceptionHandlerRedirectAddr = 0x4020ffd0;
  addSectionPtEntry(hypervisorPtd, exceptionHandlerRedirectAddr,exceptionHandlerRedirectAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  u32int blockCopyCache = gc->blockCopyCache;
  //blockCopyCache is set to Hypervisordomain in mapHypervisorMemory (See logfile:pageTablesOutput.log)

  //TODO: Check if it is possible to change accessbits for a smaller part.  Since 1 section = 1 MB which is much larger than the blockCopyCache.
  if(setAccessBits(hypervisorPtd, blockCopyCache, PRIV_RW_USR_RO)>7){
    DIE_NOW(0,"Failed to setting AccessBits for blockCopyCache");
  }
  disableCacheBit(hypervisorPtd,blockCopyCache);//Disable caching for blockCopyCache


  return hypervisorPtd;
}

descriptor* createGuestOSPageTable()
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();
  descriptor* ptd = createNew1stLevelPageTable(HYPERVISOR_FA_DOMAIN);

  mapHypervisorMemory(ptd);

  //Map Serial
  const u32int serial = SERIAL_BASE;
  if(addSmallPtEntry(ptd, serial, serial,GUEST_ACCESS_DOMAIN, GUEST_ACCESS_BITS, 0, 0, 0) != 0)
  {
    DIE_NOW(0, "Added serial mapping failed. Entering infinite loop");
  }


  /*  Map Exception vectors */

  //add section mapping for 0x14000 (base exception vectors)
  const u32int exceptionHandlerAddr = 0x14000;
  addSectionPtEntry(ptd, exceptionHandlerAddr,exceptionHandlerAddr,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //add section mapping for 0x40200000 (redirected exception vectors)
  //We will want to use the exception handler remap feature to put the page tables in the 0xffff0000 address space later
  const u32int exceptionHandlerRedirectAddr = 0x4020ffd0;
  addSectionPtEntry(ptd, exceptionHandlerRedirectAddr, exceptionHandlerRedirectAddr, HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // interrupt controller
  const u32int interruptController = 0x48200000;
  addSectionPtEntry(ptd, interruptController,interruptController,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  // gptimer1
  addSectionPtEntry(ptd, GPTIMER1,GPTIMER1,HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 0, 0, 0);

  //Map gptimer2
  const u32int gptimer2Addr = 0x49032000;
  if(addSmallPtEntry(ptd, gptimer2Addr, gptimer2Addr,HYPERVISOR_ACCESS_DOMAIN,HYPERVISOR_ACCESS_BITS, 0, 0, 0) != 0)
  {
    DIE_NOW(0, "Added gptimer2 mapping failed. Entering infinite loop");
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("New shadow PT dump @ 0x");
  serial_putint((u32int)ptd);
  serial_newline();
  dumpPageTable(ptd);
  serial_putstring("End shadow PT dump.");
#endif

  u32int blockCopyCache = gc->blockCopyCache;
  //TODO: Check if it is possible to change accessbits for a smaller part.  Since 1 section = 1 MB which is much larger than the blockCopyCache.
  //Make sure that blockCopyCache is accessible for guestOS
  if(setAccessBits(ptd, blockCopyCache, PRIV_RW_USR_RO)>7){
    DIE_NOW(0,"Failed to setting AccessBits for blockCopyCache");
  }
  disableCacheBit(ptd,blockCopyCache);//Disable caching for blockCopyCache


  return ptd;
}

descriptor* createNew1stLevelPageTable(u8int domain)
{
  //alloc some some space for the 1stLevel table
  u32int* ptd = allocMultipleFrames(CHUNKS_FOR_PAGE_TABLE, domain);

  if(0 == (u32int)ptd)
  {
    //ERROR
    DIE_NOW(0, "Frame alloc for 1st level page table failed. Entering Infinite Loop.");
  }

  //test ptd is correctly aligned
  if(((u32int)ptd & ~CHUNKS_FOR_PAGE_TABLE) != (u32int)ptd)
  {
    DIE_NOW(0, "New 1st level page table is not correctly aligned. Entering infinite loop.");
  }

#ifdef PT_SHORT_DEBUG
  serial_putstring("Page Table base addr: 0x");
  serial_putint((u32int)ptd);
  serial_newline();
#endif

  //zero it
  memset(ptd, 0, (PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH));
  return (descriptor*)ptd;
}

void sectionMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_SECTION_DEBUG
    serial_putstring("Address to 1:1 section map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addSectionPtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_SECTION_DEBUG
      serial_putstring("addSectionPtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += SECTION_SIZE;
  }
}

void smallMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("Address to 1:1 large page map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addSmallPtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += SMALL_PAGE_SIZE;
  }
}


void largeMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  while (startAddr < endAddr)
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("Address to 1:1 large map in: 0x");
    serial_putint(startAddr);
    serial_putstring(", domain: ");
    serial_putint_nozeros(domain);
    serial_putstring(", accessBits: ");
    serial_putint_nozeros(accessBits);
    serial_newline();
#endif

    u32int result = addLargePtEntry(ptd, startAddr, startAddr, domain, accessBits, c, b, tex);
    if(result)
    {
#ifdef PT_LARGE_DEBUG
      serial_putstring("addlargePtEntry failed.");
      serial_newline();
#endif
      break;
    }
    startAddr += LARGE_PAGE_SIZE;
  }
}

void mapHypervisorMemory(descriptor* ptd)
{
  //TODO: endAddr needs to be better defined
  u32int startAddr = HYPERVISOR_START_ADDR;
  u32int endAddr = startAddr + TOTAL_MACHINE_RAM/4;

  sectionMapMemory(ptd, startAddr, endAddr, HYPERVISOR_ACCESS_DOMAIN, HYPERVISOR_ACCESS_BITS, 1, 0, 0b000);
}

/* We come here first, to check that stupid things don't occur when mapping memory -> indicating potential problems */
u32int addSectionPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_SECTION_DEBUG
  serial_putstring("addSectionPtEntry. Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical addr: 0x");
  serial_putint(physical);
  serial_newline();
#endif

  /*
  First we check for any existing entries
  Before adding a new one.
  */

  descriptor* ptd1st;
  ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);
  switch(ptd1st->type)
  {
    case FAULT:
      //no existing entry
#ifdef PT_SECTION_DEBUG
      serial_putstring("Page Type: FAULT.  Creating new section descriptor");
      serial_newline();
#endif
      addNewSectionDescriptor((sectionDescriptor*)ptd1st, physical, domain, accessBits, c, b, tex);
      return 0;
      break;
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)ptd1st;
      if((sd->addr == (physical >> 20)) && (sd->sectionType == 0))
      {
        DIE_NOW(0, "pageTable.c Attempting to re-add section entry with same mapping. Entering infinite loop");

        return 0; //probably ok to continue if it's the same mapping
      }
      else
      {
        //error!
        serial_putstring("Page Table entry: ");
        serial_putint((u32int) ptd1st);
        serial_putstring("at address: 0x");
        serial_putint(*(u32int*)ptd1st);
        serial_putstring(", descriptor: 0x");
        serial_putint(*(u32int*)ptd1st);
        DIE_NOW(0, ", is already mapped.");
        return 1;
      }
      break;
    }
    case PAGE_TABLE:
      //error!
      DIE_NOW(0, "Attempting to add section entry over the top of existing small/large sub-table descriptor. Entering infinite loop.");
      return 1;
      break;
    case RESERVED:
    default:
      DIE_NOW(0, "Reserved Page table entry! Entering infinite loop.");
      return 1;
      break;
  }//switch

  //if we get here then we have not explicitly returned and want to stop here for debugging
  DIE_NOW(0, "Entering infinite loop. addSectionPtEntry");
  return 1;
}

/* Returns 0 if completed with no errors
   non zero for error?
*/
u32int addSmallPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_SMALL_DEBUG
  serial_putstring("addSmallPtEntry: Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical: 0x");
  serial_putint(physical);
  serial_putstring(", dom: ");
  serial_putint_nozeros(domain);
  serial_putstring(" AP: ");
  serial_putint_nozeros(accessBits);
  serial_putstring(" c: ");
  serial_putint_nozeros(c);
  serial_putstring(" b: ");
  serial_putint_nozeros(b);
  serial_putstring(" tex: ");
  serial_putint_nozeros(tex);
  serial_newline();
#endif
  // Two stage process
  // First we need to check for an existing 1st Level page table entry
  // If not found then create it
  // Then we check the second level
  // If found check if it is the same mapping -> true error and continue
  // found and not the same error and crash (overwrite in future?)
  // else create entry

  // retrieve 1st level entry
  descriptor* ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);
  switch(ptd1st->type)
  {
    case FAULT:
    {
      //no entry
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry: Page Type: FAULT.  Creating new descriptor");
      serial_newline();
#endif
      addNewPageTableDescriptor(ptd1st, physical, domain);
      break;
    }
    case PAGE_TABLE:
#ifdef PT_SMALL_DEBUG
      serial_putstring("addSmallPtEntry: Page Type: page table, correct.");
      serial_newline();
#endif
      // we have the correct type for a small/large pt descriptor
      break;
    case SECTION:
      serial_putstring("Page Table entry: ");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      serial_putstring(", is a section/supersection.");
      serial_newline();
      DIE_NOW(0, "Entering infinite loop.");
      break;
    case RESERVED:
    default:
      //error!
      serial_putstring("RESERVED type: 0x");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      DIE_NOW(0, "Entering infinite loop.");
  }//switch

  // At this point we know its a 2nd level page table descriptor 

  // get 2nd level table entry address
  descriptor* ptd2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)ptd1st, virtual);
#ifdef PT_SMALL_DEBUG
  serial_putstring("addSmallPtEntry: ptr to 2nd level entry: ");
  serial_putint((u32int)ptd2nd);
  serial_newline();
#endif

  // Again here we want to check the existing entry is no valid, could indicate problems elsewhere

  if(ptd2nd->type == FAULT)
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("2nd Level FAULT. Creating new small descriptor");
    serial_newline();
#endif
    addNewSmallDescriptor((smallDescriptor*)ptd2nd, physical, accessBits, c, b, tex);
  }
  else //Existing small or large page
  {
#ifdef PT_SMALL_DEBUG
    serial_putstring("Error Entry for VirtualAddress: 0x");
    serial_putint(virtual);
    serial_putstring(", already exists");
    serial_newline();
#endif

    /* Dump some more debug */
    u32int physicalAddr = physical >> 12;
    u32int ptPhyscialAddr = *(u32int*)ptd2nd >> 12;
    if(physicalAddr == (physicalAddr & ptPhyscialAddr))
    {
      //match
#ifdef PT_SMALL_DEBUG
      serial_putstring(", attempt to re-add existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_putstring("continuing.");
      serial_newline();
#endif
    }
    else
    {
      //Overwrite?
      serial_putstring(", attempt to overwrite existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_newline();
      DIE_NOW(0, "Entering infinte loop");
      return 1;
    }
  }
  return 0;
}

/* Returns 0 if completed with no errors
   non zero for error?
*/
u32int addLargePtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
#ifdef PT_LARGE_DEBUG
  serial_putstring("addLargePtEntry. Virtual Addr: 0x");
  serial_putint(virtual);
  serial_putstring(", physical: 0x");
  serial_putint(physical);
  serial_newline();
#endif
  /* Two stage process
  *First we need to check for an existing 1st Level page table entry
  *If not found then create it
  *Then we check the second level
  *If found check if it is the same mapping -> true error and continue
  *found and not the same error and crash (overwrite in future?)
  *else create entry :)
  */

  /* retrieve 1st level entry */
  descriptor* ptd1st;
  ptd1st = get1stLevelPtDescriptorAddr(ptd, virtual);

  switch(ptd1st->type)
  {
    case FAULT:
    {
      // no entry
#ifdef PT_LARGE_DEBUG
      serial_putstring("Page Type: FAULT.  Creating new descriptor");
      serial_newline();
#endif
      addNewPageTableDescriptor(ptd1st, physical, domain);
    }
    case PAGE_TABLE:
      // we have the correct type for a small/large pt descriptor
      break;
    case SECTION:
      // error!
      serial_putstring("Page Table entry: ");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);
      serial_putstring(", is a section/supersection.");
      serial_newline();
      DIE_NOW(0, "Entering infinite loop.");
      break;
    case RESERVED:
    default:
      //error!
      serial_putstring("RESERVED type: 0x");
      serial_putint((u32int) ptd1st);
      serial_putstring("at address: 0x");
      serial_putint(*(u32int*)ptd1st);

      DIE_NOW(0, "Entering infinite loop.");
      return 1;
      break;
  }//switch

  // At this point we know its a 2nd level page table descriptor */

  // Retrieve second level entry
  descriptor* ptd2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)ptd1st, virtual);

  // Again here we want to check the existing entry is no valid, could indicate problems elsewhere */

  if(ptd2nd->type == FAULT)
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("2nd Level FAULT. Creating new small descriptor");
    serial_newline();
#endif
    addNewLargeDescriptor((largeDescriptor*)ptd2nd, physical, accessBits, c, b, tex);
  }
  else //Existing small or large page
  {
#ifdef PT_LARGE_DEBUG
    serial_putstring("Error Entry for VirtualAddr: 0x");
    serial_putint(virtual);
    serial_putstring(", already exists");
    serial_newline();
#endif

    // Dump some more debug
    u32int physicalAddr = physical >> 12;
    u32int ptPhyscialAddr = *(u32int*)ptd2nd >> 12;
    if(physicalAddr == (physicalAddr & ptPhyscialAddr))
    {
      //match
#ifdef PT_LARGE_DEBUG
      serial_putstring(", attempt to re-add existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_putstring("continuing.");
      serial_newline();
#endif
    }
    else
    {
      //Overwrite?
      serial_putstring(", attempt to overwrite existing entry for PhysicalAddr: 0x");
      serial_putint(physical);
      serial_newline();
      DIE_NOW(0, "Entering infinte loop");
      return 1;
    }
  }
  return 0;
}

descriptor* get1stLevelPtDescriptorAddr(descriptor* ptd, u32int virtualAddress)
{
  /*
  ptd (page table base addr) aligned to 16KB boundary
  31             14        0
  |  Base Addr   |         |

  Virtual Addr
  31         20            0
  |  Index   |             |

  1stLevel descriptor addr
  31             14       2 1 0
  | Base Addr    | Index  |0|0|
  */
  u32int tableIndex = 0;
  tableIndex = (virtualAddress & 0xFFF00000) >> 18;
  u32int baseAddress = 0;
  baseAddress = ((u32int)ptd & 0xFFFFC000);
  descriptor* addr = (descriptor*)(tableIndex | baseAddress);
  return addr;
}

descriptor* get2ndLevelPtDescriptor(pageTableDescriptor* ptd1st, u32int virtual)
{
  /*
  ptd2nd aligned to a 1024 byte boundary
  31             10        0
  |  Base Addr   |         |

  Virtual Addr
  31   19      12          0
  |    | index  |          |

  1stLevel descriptor addr
  31             10       2 1 0
  | Base Addr    | Index  |0|0|
  */
#ifdef PT_SMALL_DEBUG
  serial_putstring("get2ndLevelPtDescriptor: ptd1st @ 0x");
  serial_putint((u32int)ptd1st);
  serial_putstring(" for vAddr ");
  serial_putint(virtual);
  serial_newline();
#endif
  u32int baseAddr = *(u32int*)ptd1st & 0xFFFFFC00; // base addr bits 31:10 -> bits 31:10
  u32int index = (virtual & 0x000FF000) >> 10; //virt addr bits 19:12 -> bits 9:2
  u32int descrAddr = baseAddr | index;
#ifdef PT_SMALL_DEBUG
  serial_putstring("get2ndLevelPtDescriptor: physical addr ptr to pt2: ");
  serial_putint(descrAddr);
  serial_newline();
#endif

  // now we have the PHYSICAL address of the 2nd level PT.
  // need virtual addr so we can access it.
  descrAddr = findVAforPA(descrAddr);

#ifdef PT_SMALL_DEBUG
  serial_putstring("get2ndLevelPtDescriptor: virtual addr ptr to pt2 ");
  serial_putint(descrAddr);
  serial_newline();
#endif
  return (descriptor*)descrAddr;
}

/*
Allocates mem, zeros & writes the new entry to the 1st level pt
return 0 for success.
*/
void addNewPageTableDescriptor(descriptor* ptd1st, u32int physical, u8int domain)
{
  //alloc 1KB for the 2nd level descriptor (256 x 32bit entries)
  descriptor* ptBaseAddr = (descriptor*)allocFrame(HYPERVISOR_FA_DOMAIN); //This gives us 4KB, fix later
  if((u32int)ptBaseAddr == 0)
  {
    DIE_NOW(0, "Memory allocation fail for the 2nd level descriptor table.");
  }

#ifdef PT_SHORT_DEBUG
  serial_putstring("Allocated memory for second level pt at addr: 0x");
  serial_putint((u32int)ptBaseAddr);
  serial_newline();
#endif

  //zero it
  memset(ptBaseAddr, 0, SECOND_LEVEL_PAGE_TABLE_SIZE);

  //create the new entry & write it
  u32int imp = 0;
  u8int sbz = 0;
  u8int ns = 0;
  u8int sbz2 = 0;
  u8int type = 1;

  u32int* entry = (u32int*)ptd1st;
  *entry = ((u32int)ptBaseAddr & 0xFFFFFC00) | imp << 9 | domain << 5 | sbz <<4 | ns <<3| sbz2 <<2 | type;

#ifdef PT_SHORT_DEBUG
  serial_putstring("Written new 1st level entry at 0x");
  serial_putint((u32int)ptd1st);
  serial_putstring(", value: 0x");
  serial_putint(*(u32int*)ptd1st);
  serial_newline();
#endif
}

void addNewSmallDescriptor(smallDescriptor* sd, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex)
{
  sd->addr = (physical >> 12);
  sd->nG=0; //using global paging
  sd->s=0; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap2= (accessBits & 0x4) >> 2;
  sd->tex=(tex & 0x7);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap10= accessBits & 0x3;  //full access
  sd->c=(c & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->b=(b & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->type= (SMALL_PAGE >> 1) & 0x1 ; //set to small page
  sd->xn=0; //execute of memory allowed
#ifdef PT_SMALL_DEBUG
  serial_putstring("Small descriptor written: 0x");
  serial_putint(*(u32int*)sd);
  serial_newline();
#endif
}

void addNewLargeDescriptor(largeDescriptor* ld, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex)
{
  ld->addr = (physical >> 16);
  ld->nG=0; //using global paging
  ld->s=0; //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->ap2= (accessBits & 0x4) >> 2;
  ld->tex=(tex & 0x7);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->ap10= accessBits & 0x3;  //full access
  ld->c=(c & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->b=(b & 0x1);  //memory sharing / caching bits B3.7.2 of ARM ARM
  ld->type=LARGE_PAGE; //set to large page
  ld->xn=0; //execute of memory allowed
#ifdef PT_LARGE_DEBUG
  serial_putstring("Large descriptor written: 0x");
  serial_putint(*(u32int*)ld);
  serial_newline();
#endif

  //Need to copy the large entry 16 times
  replicateLargeEntry(ld);
}

void addNewSectionDescriptor(sectionDescriptor* sd, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex)
{
  sd->addr= (physical >> 20);
  sd->type= 0b10; //set to section
  sd->b=(b & 0x1); //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->c=(c & 0x1); //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->xn= 0; //execute of memory allowed
  sd->domain= domain;
  sd->imp= 0; //currently unused
  sd->ap10= accessBits & 0x3;
  sd->tex=(tex & 0x7) ; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->ap2= (accessBits & 0x4) >> 2;
  sd->s= 0; //memory sharing / caching bits B3.7.2 of ARM ARM
  sd->nG= 0; //using global paging
  sd->sectionType= 0; //normal section (=1 for supersection)
  sd->ns= 1; //non secure memory?
#ifdef PT_SECTION_DEBUG
  serial_putstring("Section descriptor written: 0x");
  serial_putint(*(u32int*)sd);
  serial_newline();
#endif
}

/** Debug / entire page table dump to serial console */

void dumpPageTable(descriptor* ptd)
{
#ifdef PT_DUMP_DEBUG
  //assumes a 16KB pageTable mapped into memory
  serial_putstring("page table entries: 0x");
  serial_putint(PAGE_TABLE_ENTRIES);
  serial_newline();

  serial_putstring("VIRTUAL ADDR | TYPE | DETAILS");
  serial_newline();
  serial_putstring("-----------------------------");
  serial_newline();

  serial_putstring("ptd addr: 0x");
  serial_putint((u32int)ptd);
  serial_newline();

  descriptor* currentPtd = ptd;
  u32int i;

  for(i=0; i < PAGE_TABLE_ENTRIES; i++)
  {
    switch(currentPtd->type)
    {
      case FAULT: // 0b00
        break;
      case PAGE_TABLE: // 0b01
        dumpVirtAddr(i);
        serial_putstring("-0x");
        serial_putint(((i+1) << 20)-1);
        serial_putstring(" ");
        serial_putstring("Entry for 2nd level table: 0x");
        serial_putint( *((u32int*)currentPtd) );
        serial_newline();
        dump2ndLevel((pageTableDescriptor*)currentPtd, (i << 20));
        break;
      case SECTION: // 0b10
        dumpVirtAddr(i);
        serial_putstring(" ");
        dumpSection((sectionDescriptor*) currentPtd);
        break;
      case RESERVED: // 0b11
        dumpVirtAddr(i);
        serial_putstring(" RESERVED");
        serial_newline();
        break;
      default:
        DIE_NOW(0, "dumpPageTables: INVALID pt entry type.");
    }//switch
    currentPtd++;
  }
#else
  serial_putstring("Enable page table debug: #define PT_DUMP_DEBUG, to dump page table contents");
  serial_newline();
#endif
}

#ifdef PT_DUMP_DEBUG
void dumpVirtAddr(u32int i)
{
  serial_putstring("0x");
  serial_putint(i << 20);
}

void dump2ndVirtAddr(u32int virtual, u32int i, u32int pageSize)
{
  //Indent by 4 characters
  serial_putstring("    0x");
  switch (pageSize)
  {
    case PAGE_64KB:
      virtual += i << 16; // 16 bits -> 64KB    
      break;
    case PAGE_4KB:
      virtual += i << 12; // 12 bits -> 4KB 
      break;
    case PAGE_1KB:
      virtual += i << 10; // 10 bits -> 1KB
      break;
    default:
      DIE_NOW(0, "dump2ndVirtAddr: invalid page size.");
  } 
  serial_putint(virtual);
  serial_putstring(" ");
}

void dump2ndLevel(pageTableDescriptor* ptd, u32int virtual)
{
  serial_putstring("    SUB PAGE TABLE");
  serial_putstring(" domain ");
  serial_putint_nozeros(ptd->domain);

  u32int baseAddr = ptd->addr << 10;
  serial_putstring(" baseAddr ");
  serial_putint_nozeros(baseAddr);
  serial_newline();

  if ( (baseAddr > BEAGLE_RAM_START) &&
       (baseAddr <=BEAGLE_RAM_END-SECOND_LEVEL_PAGE_TABLE_SIZE) )
  {
    serial_putstring("    Virtual Addr | TYPE | DETAILS");
    serial_newline();
    serial_putstring("    -----------------------------");
    serial_newline();

    descriptor* currentPtd = (descriptor*) (ptd->addr << 10);
    int i = 0;
    for(i = 0; i < SECOND_LEVEL_PAGE_TABLE_ENTRIES; i++)
    {
      switch(currentPtd->type)
      {
        case FAULT:        // 0b00: translation fault (no valid entry)
          // no entry here
          break;
        case LARGE_PAGE:   // 0b01: large page (64KB)
          dump2ndVirtAddr(virtual, i, PAGE_64KB);
          dumpLargePage((largeDescriptor*)currentPtd);
          break;
        case SMALL_PAGE:   // 0b10: small page (4KB)
          dump2ndVirtAddr(virtual, i, PAGE_4KB);
          dumpSmallPage((smallDescriptor*)currentPtd);
          break;
        case SMALL_PAGE_3: // 0b11: tiny page  (1KB)
          DIE_NOW(0, "dump2ndLevel: can't handle small_page 1KB yet.");
        default:
          DIE_NOW(0, "dump2ndLevel: INVALID pt entry type.");
      }
      currentPtd++;
    }
  }
  else
  {
    serial_putstring("Address of the second level page table is not in physical RAM: 0x");
    serial_putint(baseAddr);
    serial_newline();
  }
}

void dumpSmallPage(smallDescriptor* sd)
{
  serial_putstring("SMALL PAGE");

  serial_putstring(" PhysAddr: 0x");
  serial_putint(sd->addr << 12);
  /*
  serial_putstring(" nG:");
  serial_putint_nozeros(sd->nG);
  serial_putstring(" s:");
  serial_putint_nozeros(sd->s);
  serial_putstring(" tex:");
  serial_putint_nozeros(sd->tex);
  serial_putstring(" xn:")
  serial_putint_nozeros(sd->xn);
  */
  serial_putstring(" c:");
  serial_putint_nozeros(sd->c);
  serial_putstring(" b:");
  serial_putint_nozeros(sd->b);
  serial_putstring(" accessBits:");
  u32int accessBits = (sd->ap2 << 2) | sd->ap10;
  serial_putint_nozeros(accessBits);
  serial_newline();
}

void dumpLargePage(largeDescriptor* ld)
{
  serial_putstring("LARGE PAGE");
  serial_putstring(" PhysAddr: 0x");
  serial_putint(ld->addr << 16);
  /*
  serial_putstring(" nG:");
  serial_putint_nozeros(ld->nG);
  serial_putstring(" s:");
  serial_putint_nozeros(ld->s);
  serial_putstring(" tex:");
  serial_putint_nozeros(ld->tex);
  serial_putstring(" c:");
  serial_putint_nozeros(ld->c);
  serial_putstring(" b:")
  serial_putint_nozeros(ld->b);
  serial_putstring(" xn:")
  serial_putint_nozeros(ld->xn);
  */
  serial_putstring(" accessBits:");
  u32int accessBits = (ld->ap2 << 2) | ld->ap10;
  serial_putint_nozeros(accessBits);
  serial_newline();
}

void dumpSection(sectionDescriptor* sd)
{
  if(0 != sd->sectionType)
  {
    dumpSuperSection(sd);
    return;
  }
  serial_putstring("SECTION ");
  serial_putstring(" PhysAddr: 0x");
  serial_putint(sd->addr << 20);
  serial_putstring(" domain:");
  serial_putint_nozeros(sd->domain);
  serial_putstring(" accessBits:");
  u32int accessBits = (sd->ap2) << 2 | sd->ap10;
  serial_putint_nozeros(accessBits);
  serial_putstring(" nG:");
  serial_putint_nozeros(sd->nG);
  /*
  serial_putstring(" ns:");
  serial_putint(sd->ns);
  serial_putstring(" s:");
  serial_putint(sd->s);
  serial_putstring(" tex:");
  serial_putint(sd->tex);
  serial_putstring(" imp:");
  serial_putint(sd->imp);
  serial_putstring(" xn:");
  serial_putint(sd->xn);
  serial_putstring(" c:");
  serial_putint(sd->c);
  serial_putstring(" b:");
  serial_putint(sd->b);
  */
  serial_newline();
}

void dumpSuperSection(void* sd)
{
  serial_putstring("SUPERSECTION");
  serial_newline();

  DIE_NOW(0, "UNIMPLEMENTED: dumpSuperSection (pageTable.c)");
}
#endif

void disableCacheBit(descriptor* ptd, u32int virtual)
{
  if(0 == ptd)
  {
    ptd = hypervisorPtd;
  }


  descriptor* pte = get1stLevelPtDescriptorAddr(ptd, virtual);

  switch(pte->type)
  {
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)pte;
      if(sd->sectionType != 0)
      {
        //error supersection
        DIE_NOW(0, "setting accessBits for a SuperSection is not implemented. Entering infinite loop...");
      }
      else
      {
        sd->c = 0b0;
        sd->b = 0b0;
        sd->tex = 0b100;  //TRE=0 (SCTLR= 0x00C51875) =>
        //tex[2]=1 will make it possible to chose caching behavior see p 1308 ARM ARM
        sd->s = 0;
        serial_putstring("Caching disabled");
      }
      break;
    }
    case PAGE_TABLE:
      DIE_NOW(0, "setting cacheBit for a PAGE_TABLE is not implemented. Entering infinite loop...");
    case FAULT:
      DIE_NOW(0, "setting cacheBit for a FAULT is not implemented. Entering infinite loop...");
    case RESERVED:
      DIE_NOW(0, "setting cacheBit for a RESERVED is not implemented. Entering infinite loop...");
      //fall through
    default:
      DIE_NOW(0, "setting cacheBit has done nothing. Entering infinite loop...");
    break;
  }

  //toggle imp defined bit to signify memProtection active / inactive
  pte->imp = pte->imp ^ 0x1;
  clearTLB();
  return;
}

/** Change the accessBits of a descriptor given valid accessBits & a descriptor

returns previous descriptor (value 0-7)
value > 7 is an error
*/
ACCESS_TYPE setAccessBits(descriptor* ptd, u32int virtual, ACCESS_TYPE newAccessBits)
{
  if(0 == ptd)
  {
    ptd = hypervisorPtd;
  }

  u8int currentAccessBits = 0; //We return the current access bits for the page, so they can be restored if needed

  descriptor* pte = get1stLevelPtDescriptorAddr(ptd, virtual);

#ifdef PT_DEBUG
  serial_putstring("setAccessBits: ptd=");
  serial_putint((u32int)ptd);
  serial_putstring(" virtual=");
  serial_putint(virtual);
  serial_putstring(" newAccessBits=");
  serial_putint_nozeros((u32int)newAccessBits);
  serial_newline();
  serial_putstring("setAccessBits: PTentry=");
  serial_putint((u32int)pte);
  serial_putstring(" value=");
  serial_putint(*(u32int*)pte);
  serial_newline();
#endif
  
  switch(pte->type)
  {
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)pte;
      if(sd->sectionType != 0)
      {
        //error supersection
        DIE_NOW(0, "setting accessBits for a SuperSection is not implemented. Entering infinite loop...");
      }
      else
      {
        currentAccessBits = (sd->ap2 << 2) | sd->ap10;
#ifdef PT_DEBUG
        serial_putstring("setAccessBits: Setting accessBits for SECTION page pte: 0x");
        serial_putint((u32int)pte);
        serial_putstring(" old bits: ");
        serial_putint_nozeros(currentAccessBits);
        serial_putstring(", new bits: ");
        serial_putint_nozeros(newAccessBits);
        serial_newline();
#endif
        sd->ap2 = newAccessBits >> 2;
        sd->ap10 = newAccessBits & 0x3;
      }
      break;
    }
    case PAGE_TABLE:
    {
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte, virtual);
      switch(pte2nd->type)
      {
        case SMALL_PAGE://fall through
        case SMALL_PAGE_3:
        {
          smallDescriptor* sd = (smallDescriptor*)pte2nd; //large & small pages have their access bits in the same place
          currentAccessBits = (sd->ap2 << 2) | sd->ap10;
#ifdef PT_SMALL_DEBUG
          serial_putstring("Setting accessBits for SMALL page pte: 0x");
          serial_putint((u32int)pte);
          serial_putstring(" old bits: ");
          serial_putint_nozeros(currentAccessBits);
          serial_putstring(", new bits: ");
          serial_putint_nozeros(newAccessBits);
          serial_newline();
#endif
          sd->ap2 = newAccessBits >> 2;
          sd->ap10 = newAccessBits & 3;
          break;
        }
        case LARGE_PAGE:
        {
          largeDescriptor* lpd = (largeDescriptor*)(u32int)pte2nd;
          //retrieve the current accessBits for return
          currentAccessBits = (lpd->ap2 << 2) | lpd->ap10;
#ifdef PT_LARGE_DEBUG
          serial_putstring("Setting accessBits for (16) LARGE page pte: 0x");
          serial_putint((u32int)lpd);
          serial_putstring("-0x");
          serial_putint((u32int)&lpd[15]);
          serial_putstring(" old bits: ");
          serial_putint_nozeros(currentAccessBits);
          serial_putstring(", new bits: ");
          serial_putint_nozeros(newAccessBits);
          serial_newline();
#endif
          //set the new accessBits
          lpd->ap2 = newAccessBits >> 2;
          lpd->ap10 = newAccessBits & 3;

          //copy it to the other 15 entries
          replicateLargeEntry(lpd);
          break;
        }
        case FAULT: //fall through
        default:
          DIE_NOW(0, "Error: Attempt to set accessBits for page type: FAULT, in 2nd level page table");
          return 8;
          break;
      }//switch PAGE_TABLE
      break;
    }
    case FAULT:
      DIE_NOW(0, "Error: Attempt to set accessBits for page type: FAULT!");
      return 8;
      break;
      //else fall through
    case RESERVED:
      DIE_NOW(0, "Error: Attempt to set accessBits for page type: RESERVED!");
      //fall through
    default:
      return 8; //error
    break;
  }

  //toggle imp defined bit to signify memProtection active / inactive
  pte->imp = pte->imp ^ 0x1;
  clearTLB();
  return (ACCESS_TYPE)currentAccessBits;
}


void replicateLargeEntry(largeDescriptor* ld)
{
  //looks nasty but otherwise gcc insists on using memcpy, yuck!
  u32int* toCopy = (u32int*)ld;

  //get address of first large page entry
  volatile u32int* descriptor = (u32int*) ((u32int)ld & 0xFFFFFFC0);

  u32int i;
  for(i=0; i < 16; i++)
  {
    descriptor[i] = *toCopy;
  }
}


descriptor* getPageTableEntry(descriptor* ptd, u32int address)
{
  if(0 == ptd)
  {
    DIE_NOW(0, "ptd is 0; getPageTableEntry (pageTable.c)");
  }

  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, address);
  switch(pte1st->type)
  {
    case SECTION:
    {
      if( ((sectionDescriptor*)pte1st)->sectionType == 1)
      {
        DIE_NOW(0, "SUPERSECTION not implemented");
        return 0;
      }
      else
      {
        return pte1st;
      }
      break;
    }
    case PAGE_TABLE:
    {
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, address);
      switch(pte2nd->type)
      {
        case LARGE_PAGE:
          return pte2nd;
          break;
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
          return pte2nd;
          break;
        case FAULT:
        default:
          return pte2nd;
          break;
      }
      break;
    }
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
      return pte1st;
      break;
  }
}

void copySectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow)
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();
  descriptor* guestPhysical = gc->PT_physical;

  if(guest->sectionType == 1)
  {
    // this is a 16Mb super section
    copySuperSectionEntry(guest, shadow);
  }
  else
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("Guest entry to copy: ");
    serial_newline();
    dumpSection(guest);
#endif

    /* Address mapping */
    u32int guestPhysicalAddr = guest->addr << 20;

    //Check physical address is within RAM address range
    if( (guestPhysicalAddr >= BEAGLE_RAM_START) &&
        (guestPhysicalAddr < BEAGLE_RAM_END -SECTION_SIZE +1) )
    {

      sectionDescriptor* guestReal = (sectionDescriptor*)get1stLevelPtDescriptorAddr(guestPhysical, guestPhysicalAddr);

      if(guestReal->type != SECTION)
      {
      /*
        Hitting this means we allocated guestReal memory in small/large pages
        or converted it the guestPhysical PT to smaller entries for memory protection
        Need to check if any protection is still in place, if not convert back to section map?

        This should be performed by the removeProtection code?
        And therefore make this just a check for contiguous memory
      */
#ifdef PT_SHADOW_DEBUG
        serial_putstring("PARTIAL_IMPLEMENTATION: guestPhysical contiguous memory check copySectionEntry (pageTable.c). continuing");
        serial_newline();
#endif

        /* Hack for now, really should check that the addr space is contiguous
        & remap the real physical to a section
        Also a hack in the fact we only want the top 12bits of the address,
        this casting is a bit wierd as a result of the hack
        */
        guestReal = (sectionDescriptor*)get2ndLevelPtDescriptor((pageTableDescriptor*)guestReal, guestPhysicalAddr);

        if(guestReal->type == FAULT)
        {
          DIE_NOW(gc, "Underlying address space is not mapped. copySectionEntry (pageTable.c) Entering infinite loop");
        }
      }
#ifdef PT_SHADOW_DEBUG
      u32int guestRealAddr = guestReal->addr << 20;
      serial_putstring("copySectionEntry: guest real addr: 0x");
      serial_putint(guestRealAddr);
      serial_newline();
#endif

      shadow->type = SECTION;
      shadow->addr = guestReal->addr;

      // Access control bit mapping: check if guess is not editing the PTE
      // that holds guests page table!
      u32int vAddr = (((u32int)guest - (u32int)gc->PT_os) / 4) * 1024 * 1024;
#ifdef PT_SHADOW_DEBUG
      serial_putstring("copySectionEntry: section is for VA 0x");
      serial_putint(vAddr);
      serial_newline();
#endif
      
      bool containsPTEntry = FALSE;
      if ((vAddr & 0xFFF00000) == ((u32int)gc->PT_os & 0xFFF00000))
      {
        // 1st level page table lives in this section!
        containsPTEntry = TRUE;
#ifdef PT_SHADOW_DEBUG
        serial_putstring("copySectionEntry: section contains guest 1st level page table!");
        serial_newline();
#endif
      }
      // maybe second level page tables live in this section?
      u32int metaArrayIndex = 0;
      while (guestSecondLvlPageTables[metaArrayIndex] != 0)
      {
        u32int pAddrPt2 = guestSecondLvlPageTables[metaArrayIndex];
        if( (pAddrPt2 >= guestPhysicalAddr) 
        && ((pAddrPt2 + SECOND_LEVEL_PAGE_TABLE_SIZE -1) <= (guestPhysicalAddr + SECTION_SIZE-1)) )
        {
          containsPTEntry = TRUE;
#ifdef PT_SHADOW_DEBUG
          serial_putstring("copySectionEntry: section contains guest 2nd level page table!");
          serial_newline();
#endif
        }
        metaArrayIndex++;
      }

      if (containsPTEntry)
      {
        shadow->ap2  = (PRIV_RW_USR_RO >> 2) & 0x1;
        shadow->ap10 =  PRIV_RW_USR_RO & 0x3;
      }
      else
      {
        u32int guestAPBits = (guest->ap2 << 2) | guest->ap10;
        u32int guestDomain = guest->domain;
        u32int apBits = mapAccessPermissionBits(guestAPBits, guestDomain);
  
        shadow->ap2  = (apBits >> 2) & 0x1;
        shadow->ap10 =  apBits & 0x3;
      }

      /* Check/map guest domain for access control */
      shadow->domain = mapGuestDomain(guest->domain);

      /* WARNING:Position dependant. Must be after the access control being set above! */
      //Assume this is correct, helps us with memory protection
      u8int xn = shadow->xn = guest->xn;
      if(xn == 1)
      {
        // guest maps memory as EXECUTE NEVER.
        serial_putstring("XN bit is set.  Protecting code space from self-modification block cache corruption.");
        serial_newline();
        // When memory protection is properly implemented
        // then we should point to a function that deals with this!
        //Perhaps an optimised addProtection method for the pageTable class here
        //using an extern! -> just need to add the single protection entry to the LL!
        addProtection((u32int)shadow, (u32int)shadow, 0, PRIV_RW_USR_RO);
      }
      /* End WARNING */

      //Currently just map these, may need to correct this with something proper later
      shadow->c = 0;
      shadow->b = 0;
      shadow->s = 0;
      shadow->tex = 0;
      shadow->nG = guest->nG;
      shadow->ns = guest->ns;

      if(guest->imp == 1)
      {
        // I have an idea to use the imp bit for quick memory protection in use checking,
        // without having to traverse the memory protection list/sub system
        // Unless memProtection is implemented don't worry about this at all
        serial_putstring("WARNING:guest OS PT using the imp bit.");
        serial_newline();
        serial_putstring("Please tell Virt Mem maintainer about this copySectionEntry (pageTable.c)");
        serial_newline();
      }
    }
    else if( 0x00000000 == (guestPhysicalAddr & 0xFFF0000))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if( 0x40200000 == (guestPhysicalAddr & 0xFFF00000))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x49000000 == (guestPhysicalAddr & 0xFFF00000))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
      //Leave the rest zero for now
    }
    else if( 0x48000000 == (guestPhysicalAddr & 0xFF000000))
    {
      u32int virtAddr = (u32int)shadow << 18;
      /* HACK: Map in small pages for granularity
      A proper memory protection solution would do the PT reconfiguration for use
      When the addProtection is called.
      For now map in in 4KB chunks
      */
      u32int startAddr = 0x48300000;

      if(guestPhysicalAddr == startAddr)
      {
        u32int endAddr = 0x483FFFFF;
        descriptor* shadowPTD = (descriptor*)((u32int)shadow & 0xFFFFC000);
        while (startAddr < endAddr)
        {
          u32int result = addSmallPtEntry(shadowPTD, virtAddr, startAddr, mapGuestDomain(guest->domain), PRIV_RW_USR_NO, 0, 0, 0b000);
          if(result)
          {
            DIE_NOW(0, "Hack failed: copySectionEntry (pageTable.c)");
            break;
          }
          startAddr += SMALL_PAGE_SIZE;
          virtAddr += SMALL_PAGE_SIZE;
        }

        setAccessBits(shadowPTD, 0xd830A000, PRIV_RW_USR_RO);
        /* End HACK */
      }
      else
      {
        shadow->type = SECTION;
        shadow->addr = guest->addr;
        shadow->ap10 = PRIV_RW_USR_NO & 0x3;
        shadow->ap2 = PRIV_RW_USR_NO >> 2;
        shadow->domain = mapGuestDomain(guest->domain);
      }
    }
    else if(0x54000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x55000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);

      //Leave the rest zero for now
    }
    else if(0x56000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x57000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x68000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x6C000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x6D000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else if(0x6E000000 == (guestPhysicalAddr & 0xFF000000 ))
    {
      shadow->type = SECTION;
      shadow->addr = guest->addr;
      shadow->ap10 = PRIV_RW_USR_NO & 0x3;
      shadow->ap2 = PRIV_RW_USR_NO >> 2;
      shadow->domain = mapGuestDomain(guest->domain);
    }
    else
    {
      serial_putstring("Write handler for guestPhysical mapping to non RAM range.");
      serial_newline();
      serial_putstring("copySectionEntry (pageTable.c)");
      serial_newline();
      serial_putstring("Guest Physical Address: 0x");
      serial_putint(guestPhysicalAddr);
      serial_newline();
      DIE_NOW(0, "Entering infinite loop...");
    }
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("Shadow entry after copy: ");
  serial_newline();
  dumpSection(shadow);
#endif

}

void copySuperSectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow)
{
  DIE_NOW(0, "UNIMPLEMENTED: copySuperSectionEntry (pageTable.c)");
}

void copyPageTableEntry(pageTableDescriptor* guest, pageTableDescriptor* shadow)
{
  GCONTXT* gc = getGuestContext();

  // STEP 1: allocate a new frame for a new second level page table.
  u32int *newFrame = allocFrame(HYPERVISOR_FA_DOMAIN);
  if (newFrame == 0x0)
  {
    DIE_NOW(0, "frameAllocator returned null ptr. copyPageTableEntry (pageTable.c)");
  }
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTableEntry: guest ");
  serial_putint((*(u32int*)guest));
  serial_putstring(" shadow ");
  serial_putint((*(u32int*)shadow));
  serial_putstring(" newFrameAddr ");
  serial_putint((u32int)newFrame);
  serial_newline();
#endif

  // STEP 2: zero the new shadow second level page table
  memset(newFrame, 0, SECOND_LEVEL_PAGE_TABLE_SIZE);

  // STEP 3: fill in the given 1st level shadow page table entry with correct data 
  shadow->addr = (u32int)newFrame >> 10;
  //This is just a copy of the high level descriptor
  shadow->type = PAGE_TABLE;
  shadow->domain = mapGuestDomain(guest->domain);
  shadow->ns = guest->ns;

  //If the guest addr ptr to the 2nd level pt is valid, then copy it
  u32int phyAddr = getPhysicalAddress(gc->PT_physical, (guest->addr << 10));
  
  u32int metaArrayIndex = 0;
  while (guestSecondLvlPageTables[metaArrayIndex] != 0)
  {
    metaArrayIndex++;
  }
  guestSecondLvlPageTables[metaArrayIndex] = phyAddr;
  
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTableEntry: guest 2nd level PT PA ");
  serial_putint(phyAddr);
  serial_putstring(" VA ");
  serial_putint((guest->addr << 10));
  serial_newline();
#endif

  // STEP 4: copy entries from guest 2nd level PT to shadow 2nd level PT
  // STEP 4a: get guest 2nd level page table address (it is physical) from
  // first level entry
  u32int guestPageTableAddr = guest->addr << 10;
  
  // STEP 4b: map this physical address 1-2-1 to virtual address (in 1st lvl SPT)
  // for that: calculate address in the shadow page table of the entry
  // where we must add a 1-2-1 mapping
  u32int entryAddress = guestPageTableAddr >> 20;
  entryAddress = entryAddress * 4 + (u32int)gc->PT_shadow;
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTableEntry: 1-2-1 entry address ");
  serial_putint(entryAddress);
  serial_newline();
#endif
  // save old entry from that address whatever it is, and make a new entry
  u32int oldEntry = *((u32int*)entryAddress);
  u32int newEntry = ((guest->addr << 10) & 0xFFF00000) | 0x805E2;
  *(u32int*)entryAddress = newEntry;

  // STEP 4c: copy entries from guest 2nd lvl PT to shadow 2nd lvl PT
  descriptor* guestPte  = (descriptor*)((guest->addr << 10));
  descriptor* shadowPte  = (descriptor*)newFrame;
  u32int i = 0;
  for (i=0; i < SECOND_LEVEL_PAGE_TABLE_ENTRIES; i++)
  {
    if (guestPte[i].type != FAULT)
    {
      if(guestPte[i].type == LARGE_PAGE)
      {
        copyLargeEntry((largeDescriptor*)&guestPte[i], (largeDescriptor*)&shadowPte[i]);
      }
      else
      {
        copySmallEntry((smallDescriptor*)&guestPte[i], (smallDescriptor*)&shadowPte[i]);
      }
    }
  }
  
  // STEP 4d: remove temporary 1-2-1 mapping restore old entry in 1st lvl sPT
  *(u32int*)entryAddress = oldEntry;

  // STEP 5: done copying. Now find all virtual mappings to this physical addr
  // and add protection.
  // TODO: this only finds first VA mapping ot PA. this can be many-2-one, so find all..
  u32int virtAddr = findVAforPA(phyAddr);

  u32int res = addProtection(virtAddr, virtAddr+1023, 0, PRIV_RW_USR_RO);
  if (res > 7)
  {
    DIE_NOW(0, "copyPageTableEntry: failed to add memory protection.");
  }
}

void copyLargeEntry(largeDescriptor* guest, largeDescriptor* shadow)
{
#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyLargeEntry(guest=");
  serial_putint(*(u32int*)guest);
  serial_putstring(", shadow=");
  serial_putint(*(u32int*)shadow);
  serial_newline();
#endif
  DIE_NOW(0, "UNIMPLEMENTED: copyLargeEntry (pageTable.c)");
}

void copySmallEntry(smallDescriptor* guest, smallDescriptor* shadow)
{
  GCONTXT* gc = (GCONTXT*)getGuestContext();

#ifdef PT_SHADOW_DEBUG
  serial_putstring("copySmallEntry: guestEntry @ ");
  serial_putint((u32int)guest);
  serial_putstring(" = ");
  serial_putint(*(u32int*)guest);
  serial_putstring("; corresponding shadow entry @ ");
  serial_putint((u32int)shadow);
  serial_putstring(" = ");
  serial_putint(*(u32int*)shadow);
  serial_newline();
#endif

  // Address mapping: guest entry points to guest physical. Translate to host physical
  u32int guestPA = guest->addr << 12;

  //Check physical address is within RAM address range
  if ( (guestPA >= BEAGLE_RAM_START) && (guestPA < (BEAGLE_RAM_END - SECTION_SIZE + 1)) )
  {
    descriptor* hostPAentry = (descriptor*)get1stLevelPtDescriptorAddr(gc->PT_physical, guestPA);

    u32int hostPA = 0;
    switch (hostPAentry->type)
    {
      case SECTION:
      {
        sectionDescriptor* hostPAsection = (sectionDescriptor*)hostPAentry;
        hostPA = ((hostPAsection->addr << 20) & 0xFFF00000) | (guestPA & 0x000FFFFF);
        break;
      }
      case PAGE_TABLE:
      {
        descriptor* hostPAsecond =  
          get2ndLevelPtDescriptor((pageTableDescriptor*)hostPAentry, guestPA);
        switch (hostPAsecond->type)
        {
          case SMALL_PAGE:
          {
            smallDescriptor* hostPAsmall = (smallDescriptor*)hostPAsecond;
            hostPA = (hostPAsmall->addr << 12) & 0xFFFFF000;
            break;
          }
          case SMALL_PAGE_3:
          case LARGE_PAGE:
          case FAULT:
          default:
            DIE_NOW(0, "copySmallEntry: hostPA second level entry unimplemented.");
        }
        break;
      }
      case FAULT:
      case RESERVED:
      default:
        serial_putstring("copySmallEntry: gc->PT_physical = 0x");
        serial_putint((u32int)gc->PT_physical);
        serial_putstring(" hostPAentry @ 0x");
        serial_putint((u32int)hostPAentry);
        serial_putstring(" is ");
        serial_putint(*(u32int*)hostPAentry);
        serial_newline();
        DIE_NOW(0, "copySmallEntry: invalid entry found translating guestPA to hostPA.");
    }


    shadow->type = 1; // small page entry.
    shadow->addr = (hostPA >> 12) & 0xFFFFF;

    // get virtual address that this small page maps:
    u32int vAddr = 0;
    u32int maskedAddr = (u32int)shadow & 0xFFFFFC00;
    u32int ptIndex = 0;
    for (ptIndex = 0; ptIndex < PAGE_TABLE_ENTRIES; ptIndex++)
    {
      descriptor* ptEntry = &gc->PT_shadow[ptIndex];
      if (ptEntry->type == PAGE_TABLE)
      {
        pageTableDescriptor* ptDesc = (pageTableDescriptor*)ptEntry;
        if ( (((u32int)ptDesc->addr << 10) & 0xFFFFFC00) == maskedAddr)
        {
          vAddr = ptIndex << 20;  
          break;
        } 
      }
    }
    if (vAddr == 0)
    {
      DIE_NOW(gc, "copySmallEntry: couldn't find VA corresponding to edit.");
    }

#ifdef PT_SHADOW_DEBUG
    serial_putstring("copySmallEntry: small page is for VA 0x");
    serial_putint(vAddr);
    serial_newline();
#endif
    
    bool containsPTEntry = FALSE;
    if ((vAddr & 0xFFF00000) == ((u32int)gc->PT_os & 0xFFF00000))
    {
      // 1st level page table lives in this section!
      DIE_NOW(gc, "copySmallEntry: may contain guest 1st level page table!");
    }

    // maybe second level page tables live in this section?
    u32int metaArrayIndex = 0;
    while (guestSecondLvlPageTables[metaArrayIndex] != 0)
    {
      u32int pAddrPt2 = guestSecondLvlPageTables[metaArrayIndex];
      if( (pAddrPt2 >= guestPA) 
      && ((pAddrPt2 + SECOND_LEVEL_PAGE_TABLE_SIZE -1) <= (guestPA + SECTION_SIZE-1)) )
      {
        containsPTEntry = TRUE;
#ifdef PT_SHADOW_DEBUG
        serial_putstring("copySmallEntry: page contains guest 2nd level page table!");
        serial_newline();
#endif
        break;
      }
      metaArrayIndex++;
    }
    if (containsPTEntry)
    {
      shadow->ap2  = (PRIV_RW_USR_RO >> 2) & 0x1;
      shadow->ap10 =  PRIV_RW_USR_RO & 0x3;
    }
    else
    {
      // get domain from first level entry, at index found earlier: ptIndex
      pageTableDescriptor* ptEntry = (pageTableDescriptor*)&gc->PT_os[ptIndex];
      
      u32int guestAPBits = (guest->ap2 << 2) | guest->ap10;
      u32int guestDomain = ptEntry->domain;
      u32int apBits = mapAccessPermissionBits(guestAPBits, guestDomain);

      shadow->ap2  = (apBits >> 2) & 0x1;
      shadow->ap10 =  apBits & 0x3;
    }

    /* WARNING:Position dependant. Must be after the access control being set above! */
    //Assume this is correct, helps us with memory protection
    u8int xn = shadow->xn = guest->xn;
    if(xn == 1)
    {
      // guest maps memory as EXECUTE NEVER.
      serial_putstring("XN bit is set.  Protecting code space from self-modification block cache corruption.");
      serial_newline();
      // When memory protection is properly implemented
      // then we should point to a function that deals with this!
      //Perhaps an optimised addProtection method for the pageTable class here
      //using an extern! -> just need to add the single protection entry to the LL!
      addProtection((u32int)shadow, (u32int)shadow, 0, PRIV_RW_USR_RO);
    }
    /* End WARNING */

    //Currently just map these, may need to correct this with something proper later
    shadow->c = 0;
    shadow->b = 0;
    shadow->s = 0;
    shadow->tex = 0;
    shadow->nG = guest->nG;
  }
  else
  {
    // mapping some peripheral in small pages?
    serial_putstring("copySmallEntry: guest *(");
    serial_putint((u32int)guest);
    serial_putstring(")=");
    serial_putint(*(u32int*)guest);
    serial_putstring("; shadow *(");
    serial_putint((u32int)shadow);
    serial_putstring(")=");
    serial_putint(*(u32int*)shadow);
    serial_newline();
    serial_putstring("copySmallEntry: guestPA = ");
    serial_putint(guestPA);
    serial_newline();
    DIE_NOW(0, "copySmallEntry: mapping non-ram physical address, unimplemented.");
  }
}

void splitSectionToSmallPages(descriptor* ptd, u32int vAddr)
{
#ifdef PT_DEBUG
  serial_putstring("splitSectionToSmallPages: 1st level PT @ ");
  serial_putint((u32int)ptd);
  serial_newline();
#endif
  // 1. get section entry
  sectionDescriptor* sectionEntryPtr = (sectionDescriptor*)get1stLevelPtDescriptorAddr(ptd, vAddr);
  // 2. invalidate entry. (in the future - implement and call removePTEntry method)
  sectionEntryPtr->type = 0;
  // 3. map memory in small pages
  vAddr = vAddr & 0xFFF00000;
  u32int pAddr = sectionEntryPtr->addr;
  pAddr = pAddr << 20;
  
#ifdef PT_DEBUG
  serial_putstring("splitSectionToSmallPages: vaddr ");
  serial_putint(vAddr);
  serial_putstring(", pAddr ");
  serial_putint(pAddr);
  serial_newline();
#endif

  u32int protectionBits = sectionEntryPtr->ap10;
  u32int domainBits = sectionEntryPtr->domain;
  u32int index = 0;
  for (index = 0; index < 256; index++)
  {
    addSmallPtEntry(ptd, vAddr+(SMALL_PAGE_SIZE*index), pAddr+(SMALL_PAGE_SIZE*index), domainBits, protectionBits, 0, 0, 0);
  }
}      



u32int findVAforPA(u32int physAddr)
{
  GCONTXT* gc = getGuestContext();
  u32int mask = 0xFFF00000;
  u32int masked = physAddr & mask;
  
  if (!isMmuEnabled())
  {
    return physAddr;
  }

  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;

  int i = 0;
  for (i = 0; i <= PAGE_TABLE_ENTRIES; i++)
  {
    u32int entryAddress = (u32int)ptd + i*4; // entry size = 1 word
    u32int entryAtIndex = *(u32int*)entryAddress;

    u32int type = entryAtIndex & 0x3;
    switch(type)
    {
      case SECTION:
        if ((entryAtIndex & mask) == masked) 
        {
          u32int VA = i * 1024 * 1024;
          return VA+(physAddr & 0x000FFFFF);
        }
        break;
      case PAGE_TABLE:
      {
        u32int VA = 0;
        // this is more tricky... this the gives PA address of 2nd level table
        // but we need to access it, and cant do it via physical address!

        // 1. get guest 2nd level page table physical address from 1st level entry
        u32int guestPageTableAddr = entryAtIndex & 0xfffffc00;

        // 2. map this physical address 1-2-1 to virtual address (in 1st lvl SPT)
        // for that: calculate address in the shadow page table of the entry
        // where we must add a 1-2-1 mapping
        u32int tempEntryAddr = guestPageTableAddr >> 20;
        tempEntryAddr = tempEntryAddr * 4 + (u32int)ptd;

        // 4. save old entry from that address whatever it is, and make a new entry
        u32int oldEntry = *((u32int*)tempEntryAddr);
        u32int newEntry = (guestPageTableAddr & 0xFFF00000) | 0x805E2;
        *(u32int*)tempEntryAddr = newEntry;
        
        // 5. look through 2nd level page table
        descriptor* pt2Entry  = (descriptor*)guestPageTableAddr;
        u32int y = 0;
        for (y=0; y < SECOND_LEVEL_PAGE_TABLE_ENTRIES; y++)
        {
          if (pt2Entry[y].type == LARGE_PAGE)
          {
            DIE_NOW(gc, "findVAforPA: found large page!");
          }
          else if ((pt2Entry[y].type == SMALL_PAGE) || (pt2Entry[y].type == SMALL_PAGE_3))
          {
            u32int paMapped = (*(u32int*)pt2Entry) & 0xFFFFF000;
            u32int paGiven = physAddr & 0xFFFFF000;
            if (paMapped == paGiven)
            {
              // found second level entry small page that maps this phys addr
              // form VA: top megabyte from first level entry
              VA = i * 1024 * 1024;
              // index Y maps 4kB page inside this megabyte
              VA = VA | (y * 4096);
              // and we need index in the page from given physAddr
              VA = VA | (physAddr & 0xFFF);
              // bingo, we have the virtual address for the given physical address
              // restore the old entry
              *(u32int*)tempEntryAddr = oldEntry;
              return VA;
            } // if found match
          } // small page comparison
        } // for loop - 2nd level page table
        
        // 6. remove temporary 1-2-1 mapping restore old entry in 1st lvl sPT
        *(u32int*)tempEntryAddr = oldEntry;
      }
      case FAULT:
      case RESERVED:
      default:
        break;
    } // switch ends
  }
  DIE_NOW(gc, "findVAforPA: could not find virtual address");
  return 0;
}

// check if a virtual address is in any of the guest page tables.
bool isAddrInGuestPT(u32int vaddr)
{
  GCONTXT* gc = getGuestContext();

  if (gc->PT_os == 0)
  {
    // no guest page table yet.
    return FALSE;
  }

  // is the address in guests first level page table?
  if((vaddr >= (u32int)gc->PT_os) && (vaddr < ((u32int)gc->PT_os + PAGE_TABLE_SIZE - 3)) ) //16KB
  {
    return TRUE;
  }
  
  // if not in first level page table, maybe in some second level page table?
  u32int metaArrayIndex = 0;
  while (guestSecondLvlPageTables[metaArrayIndex] != 0)
  {
    // found a second level guest page table.
    u32int paddrSecond = guestSecondLvlPageTables[metaArrayIndex];
    u32int paddrGiven = getPhysicalAddress(gc->PT_os, vaddr);
    if((paddrGiven >= paddrSecond) && (paddrGiven < (paddrSecond + SECOND_LEVEL_PAGE_TABLE_SIZE - 3)) )
    {
      return TRUE;
    }
    metaArrayIndex++;
  }
  // otherwise, not found - return false
  return FALSE; 
}


/** Copy the entries from the guestVirtual -> guestPhysical -> realPhysical
Takes three page Tables:
* the shadow destination PT
* the guest OS PT
* the hypervisor PT for guest Physical to real physical mapping

Note this is a VERY expensive operation!

*/
void copyPageTable(descriptor* guest, descriptor* shadow)
{
  if(shadow == 0)
  {
    DIE_NOW(0, "Shadow pageTable is null. copyPageTable (pageTable.c) Entering infinite loop...");
  }

  if(guest == 0)
  {
    DIE_NOW(0, "guest pageTable is null. copyPageTable (pageTable.c) Entering infinite loop...");
  }

#ifdef PT_SHADOW_DEBUG
  serial_putstring("copyPageTable: Dumping guest pagetable");
  serial_newline();
  dumpPageTable(guest);
  serial_putstring("copyPageTable: End guest pageTable dump");
  serial_newline();
#endif

  //loop over the guest PT
  u32int i;
  for (i=0; i < PAGE_TABLE_ENTRIES; i++)
  {
    /******************************************************************************
     * Page table spec does not mandate that bits 31-2 are clear
     * This could fail if the guest OS does not clear the entry when removing it
     * (i.e. rather than writing 32bit zero to the entry
     * the OS just clears/sets the type bit field to 0b00)
     * and we should do the relativly more expensive test (currentPTE->type != 0)
     *****************************************************************************/

    if ((guest[i].type == SECTION) || (guest[i].type == PAGE_TABLE))
    {
      /*
       This (guest page table mapping hypervisor memory space) could
       cause serious problems if not caught early
      */
      const u32int hypervisorStart = (HYPERVISOR_START_ADDR) >> 20;
      const u32int hypervisorEnd = (HYPERVISOR_START_ADDR + (TOTAL_MACHINE_RAM/4) -1) >> 20;
      if ((hypervisorStart <= i) && (i <= hypervisorEnd))
      {
        serial_putstring("guest is attemping to use hypervisor 1:1 mapped address space. 0x");
        serial_putint(i << 20);
        serial_newline();
        DIE_NOW(0, "Entering infinite loop...");
      }

      if(guest[i].type == SECTION)
      {
        sectionDescriptor* guestSd = (sectionDescriptor*) &guest[i];
        sectionDescriptor* shadowSd = (sectionDescriptor*) &shadow[i];

        //Supersection check is done inside copySectionEntry
        copySectionEntry(guestSd, shadowSd);
      }
      else if (guest[i].type == PAGE_TABLE)
      {
        pageTableDescriptor* guestSd  = (pageTableDescriptor*) &guest[i];
        pageTableDescriptor* shadowSd = (pageTableDescriptor*) &shadow[i];

        copyPageTableEntry(guestSd, shadowSd);
      }//else !SECTION
    }// if !FAULT
#ifdef PT_SHADOW_DEBUG
    else if (guest[i].type == RESERVED)
    {
      //Not going to copy this entry.  The guest will never get to read the sPT, so no need to.
      //Whatever reason the guest is using the RESERVED entry for!
      serial_putstring("RESERVED entry in shadow page table copyPageTable (pageTable.c). continuing");
      serial_newline();
    }
#endif
  }//for loop 1st level page table
}

/*
  Needs putting somewhere more appropriate, perhaps its own file
  once we implement guest domain mapping (held in CP15?/guestContext?)
*/
u8int mapGuestDomain(u8int guestDomain)
{
#ifdef PT_SHADOW_DEBUG
  serial_putstring("UNIMPLEMENTED: mapGuestDomain (cp15coproc.c). continuing");
  serial_newline();
#endif
#ifdef PT_DOMAIN_DEBUG
  if(guestDomain == HYPERVISOR_ACCESS_DOMAIN)
  {
    serial_putstring("guestDomain is same as hypervisor domain. mapGuestDomain (pageTable.c)");
    serial_newline();
  }
#endif

  return GUEST_ACCESS_DOMAIN;
}

u32int getPageEndAddr(descriptor* ptd, u32int address)
{
  //Get the PT type, mask the virtual address, return the end addr of the page
  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, address);
#ifdef PT_DEBUG
  serial_putstring("getPageEndAddr: ptd=");
  serial_putint((u32int)ptd);
  serial_putstring(" addr=");
  serial_putint(address);
  serial_putstring(" PTentry=");
  serial_putint((u32int)pte1st);
  serial_putstring(" val=");
  serial_putint(*(u32int*)pte1st);
  serial_newline();
#endif

  u32int mask;

  switch(pte1st->type)
  {
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)pte1st;
      if(1 == sd->sectionType)
      {
        //SUPERSECTION
        DIE_NOW(0, "UNIMPLEMENTED: getPageEndAddr for SUPERSECTION. Entering infinite loop");
        return 0; // error
      }
      else
      {
        mask = 0xFFF00000;
        address = address & mask;
        address += SECTION_SIZE -1;
        return address;
      }
      break;
    }
    case PAGE_TABLE:
    {
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, address);

      switch(pte2nd->type)
      {
        case LARGE_PAGE:
        {
          mask = 0xFFFF0000;
          address = address & mask;
          address += LARGE_PAGE_SIZE -1;
          return address;
          break;
        }
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
        {
          mask = 0xFFFFF000;
          address = address & mask;
          address += SMALL_PAGE_SIZE -1;
          return address;
          break;
        }
        case FAULT: //fall through
        default:
#ifdef PT_SHORT_DEBUG
          serial_putstring("Error: getPageEndAddr FAULT/RESERVED (pageTable.c)");
          serial_newline();
#endif
          return 0; //This is an impossible value, so good for errors
          break;
      }
      break;
    }
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
#ifdef PT_SHORT_DEBUG
      serial_putstring("Error: getPageEndAddr FAULT/RESERVED (pageTable.c)");
      serial_newline();
#endif
      return 0; //This is an impossible value, so good for errors
      break;
  }
}


/* Called from the instruction emulator when we have a write privilege fault */
void pageTableEdit(u32int address, u32int newVal)
{
#ifdef PT_DEBUG
  serial_putstring("PageTableEdit: address ");
  serial_putint(address);
  serial_putstring(" newval: ");
  serial_putint(newVal);
  serial_newline();
#endif

  GCONTXT* gc = (GCONTXT*)getGuestContext();

  u32int virtualAddr;
  bool firstLevelEntry;

  // Step 1: Get the virtual fault address

  //If we faulted on a write to the 1st level PT then we already have the virtual address
  if( (u32int)gc->PT_os == (address & 0xFFFFC000) )
  {
    firstLevelEntry = TRUE;
    // get the virtual address by shifting bits 13:2 of the FA to be bits 31:20 of the VA
    virtualAddr = address << 18;
  }
  else
  {
    // write to an existing 2nd Level table, which we now need to find
    // to get the full virtual address
    firstLevelEntry = FALSE;

    //The first level PT contains a ptr to the physical address of the (1KB) 2nd level PT.
    //We have the address of the 2nd level PT, but don't know which segment of virt mem it serves
    //Address to 2nd level PT that we want to match (held in the PageTable struct so its right shifted)
    descriptor* pteAddr = get1stLevelPtDescriptorAddr(gc->PT_os, address);
    u32int pte = 0;
    switch (pteAddr->type)
    { 
      case FAULT:
        DIE_NOW(0, "pageTableEdit: fault entry hit - ?");
        break;
      case PAGE_TABLE:
        DIE_NOW(0, "pageTableEdit: second level entry hit - unimplemented.");
        break;
      case SECTION:
      {
        pte = (((sectionDescriptor*)pteAddr)->addr) << 20; 
#ifdef PT_SHADOW_DEBUG
        serial_putstring("1st level page table section entry @ ");
        serial_putint((u32int)pteAddr);
        serial_putstring("=");
        serial_putint(pte);
        serial_newline();
#endif
        break;
      }
      case RESERVED:
        DIE_NOW(0, "pageTableEdit: reserved entry hit - ?");
        break;
      default:
        break;
    } // switch ends

    // translate faulting address to physical address
    u32int faultPA = pte | (address & 0x000FFFFF);
    faultPA = faultPA & 0xFFFFFC00;

#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: 2nd level entry we are looking for: 0x");
    serial_putint(faultPA);
    serial_newline();
#endif

    u32int gPageTableBase = (u32int)gc->PT_os;
    // loop over the entire GUEST 1st level PT 
    // looking for a match to the 2nd level base address.
    u32int i;
    for(i=0;i < PAGE_TABLE_ENTRIES; i++)
    {
      u32int lvl1entry = *(u32int*)(gPageTableBase + i*4);
      if ( ((lvl1entry & 0x3) == PAGE_TABLE) &&
           ((lvl1entry & 0xFFFFFC00) == faultPA) )
      {
#ifdef PT_SHADOW_DEBUG
        serial_putstring("pageTableEdit: found entry at index ");
        serial_putint_nozeros(i);
        serial_putstring(" = ");
        serial_putint(lvl1entry);
        serial_newline();
#endif
        break;
      }
    }

    //Now have the first 12bits of the virtual address bits 31:20
    //and the bits 19:12 of the virtual address are from bits 10:2 of the fault address
    virtualAddr = (i << 20) | ((address & 0x3FC) << 10);
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: formed VA of gPT2: ");
    serial_putint(virtualAddr);
    serial_newline();
#endif
  }

  // virtualAddr points to the shadow 1st level page table entry for the faulting addr
  // get this entry from the shadow table
  descriptor* shadowEntry = get1stLevelPtDescriptorAddr(gc->PT_shadow, virtualAddr);
  descriptor* oldGuestEntry = (descriptor*)address;
  descriptor* newGuestEntry = (descriptor*)&newVal;


#ifdef PT_SHADOW_DEBUG
  if (newVal != 0)
  {
    serial_putstring("pageTableEdit: addr ");
    serial_putint(address);
    serial_putstring(" newVal ");
    serial_putint(newVal);
    serial_putstring(" virtualAddr ");
    serial_putint(virtualAddr);
    serial_newline();
    serial_putstring("pageTableEdit: shadowEntry ");
    serial_putint((*(u32int*)shadowEntry));
    serial_putstring(" oldGuestEntry ");
    serial_putint((*(u32int*)oldGuestEntry));
    serial_putstring(" newGuestEntry ");
    serial_putint((*(u32int*)newGuestEntry));
    serial_newline();
  }
#endif

  //early exit if OS is not actually updating anything
  if (*(u32int*)oldGuestEntry == newVal)
  {
    return;
  }

  if (oldGuestEntry->type == RESERVED)
  {
    DIE_NOW(gc, "pageTableEdit: old entry RESERVED type");
  }

  // Step 2: Work Out what kind of change we are dealing with

  // CASE 1: Remove, or change entry type (Remove & Add) 
  // Changing pte type if not from !Fault to Fault, is a remove then add operation
  if ((oldGuestEntry->type != newGuestEntry->type) && (oldGuestEntry->type != FAULT) )
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: remove/change type case");
    serial_newline();
#endif

    if (firstLevelEntry)
    {
      // first level page table edit
      if (oldGuestEntry->type == PAGE_TABLE)
      {
        // changing a page table to a section or fault, need to remove PT entry.
        removePageTableEntry((pageTableDescriptor*)shadowEntry);
      }
      else if (oldGuestEntry->type == SECTION)
      {
        // changing a page table to a section or fault, need to remove PT entry.
        removeSectionEntry((sectionDescriptor*)shadowEntry);
      }
      // nothing to do if old type was reserved.
    }
    else
    {
      // second level page table edit
      if (oldGuestEntry->type == LARGE_PAGE)
      {
        // changing a page table to a section or fault, need to remove PT entry.
        removeLargePageEntry((largeDescriptor*)shadowEntry);
      }
      else if (oldGuestEntry->type == SMALL_PAGE || newGuestEntry->type == SMALL_PAGE_3)
      {
        // changing a page table to a section or fault, need to remove PT entry.
        removeSmallPageEntry((smallDescriptor*)shadowEntry);
      }
    }
  }

  // CASE 2: add. follows 'change type' as change type is 'remove then add'
  if( (newGuestEntry->type != FAULT) && (oldGuestEntry->type != newGuestEntry->type) )
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit: add entry case");
    serial_newline();
#endif

    if(firstLevelEntry)
    {
      if(newGuestEntry->type == SECTION)
      {
        copySectionEntry((sectionDescriptor*)newGuestEntry, (sectionDescriptor*)shadowEntry);
      }
      else if (newGuestEntry->type == PAGE_TABLE)
      {
        copyPageTableEntry((pageTableDescriptor*)newGuestEntry, (pageTableDescriptor*)shadowEntry);
      }
      else
      {
        // adding a reserved entry.
        shadowEntry->type = RESERVED;
        clearTLBbyMVA(address);
        return;
      }
    }
    else
    {
      // to copySmallEntry and copyLargeEntry:
      // need to get second level page table address, not first level PTE
      u32int secondLvlEntryAddr = (*(u32int*)shadowEntry) & 0xFFFFFC00;
      secondLvlEntryAddr = secondLvlEntryAddr | ((virtualAddr >> 10) & 0x3FC);
      if(newGuestEntry->type == LARGE_PAGE)
      {
        copyLargeEntry((largeDescriptor*)newGuestEntry, (largeDescriptor*)secondLvlEntryAddr);
      }
      else if(newGuestEntry->type == SMALL_PAGE || newGuestEntry->type == SMALL_PAGE_3)
      {
        copySmallEntry((smallDescriptor*)newGuestEntry, (smallDescriptor*)secondLvlEntryAddr);
      }
    }
  }
  

  // CASE 3: change entry details
  if (oldGuestEntry->type == newGuestEntry->type)
  {
#ifdef PT_SHADOW_DEBUG
    serial_putstring("pageTableEdit, EDIT entry case @ 0x");
    serial_putint(address);
    serial_putstring(", oldValue: 0x");
    serial_putint(*(u32int*)address);
    serial_putstring(", newValue: 0x");
    serial_putint(newVal);
    serial_putstring(", shadowEntry: 0x");
    serial_putint(*(u32int*)shadowEntry);
    serial_newline();
#endif

    if (firstLevelEntry)
    {
      // first level entry
      if (oldGuestEntry->type == SECTION)
      {
        sectionDescriptor* oldSd = (sectionDescriptor*)oldGuestEntry;
        sectionDescriptor* newSd = (sectionDescriptor*)newGuestEntry;
        // WARNING:  shadow descriptor type might not correspond to guest descriptor type!!! 
        sectionDescriptor* shadowSd = (sectionDescriptor*)shadowEntry;
        if( (oldSd->sectionType != newSd->sectionType) || (oldSd->addr != newSd->addr) )
        {
          // if changing base address or section <-> supersection
          // then it's a remove&add operation
          removeSectionEntry((sectionDescriptor*)shadowEntry);
          copySectionEntry((sectionDescriptor*)newGuestEntry, (sectionDescriptor*)shadowEntry);
        }
        // now changing misc PTE details
        // we so far ignore: C, B, TEX, s, nG, ns bits!!!

        if (oldSd->xn != newSd->xn)
        {
          DIE_NOW(gc, "pageTableEdit: edit section PTE details XN bit toggle!");
        }
        
        // important: first remap domain, then access permission bits!!!
        shadowSd->domain = mapGuestDomain(newSd->domain);

        if(oldSd->xn != newSd->xn)
        {
          shadowSd->xn = newSd->xn;
        }
  
        //Carefull of this one, field is used by the hypervisor
        if(oldSd->imp != newSd->imp )
        {
          //If we hit this then I need to rethink how we check for memory protection being set on a page before modification
          //As the guestOS is now using the field!
          DIE_NOW(0, "pageTableEdit: edit case: pageTable, Use of implementation defined field. Check what guestOS does with this as the Hypervisor uses it!");
        }

        if ((oldSd->ap10 != newSd->ap10) || (oldSd->ap2 != newSd->ap2))
        {
          // Access control bit mapping: check if guess is not editing the PTE
          // that holds guests page table!
          bool containsPTEntry = FALSE;
          if ((virtualAddr & 0xFFF00000) == ((u32int)gc->PT_os & 0xFFF00000))
          {
            // 1st level page table lives in this section!
            containsPTEntry = TRUE;
          }
          // maybe second level page tables live in this section?
          u32int guestPhysicalAddr = getPhysicalAddress(gc->PT_os, virtualAddr);
          u32int metaArrayIndex = 0;
          while (guestSecondLvlPageTables[metaArrayIndex] != 0)
          {
            u32int pAddrPt2 = guestSecondLvlPageTables[metaArrayIndex];
            if( (pAddrPt2 >= guestPhysicalAddr) 
            && ((pAddrPt2 + SECOND_LEVEL_PAGE_TABLE_SIZE -1) <= (guestPhysicalAddr + SECTION_SIZE-1)) )
            {
              containsPTEntry = TRUE;
              break;
            }
            metaArrayIndex++;
          }
  
          if (containsPTEntry)
          {
            shadowSd->ap2  = (PRIV_RW_USR_RO >> 2) & 0x1;
            shadowSd->ap10 =  PRIV_RW_USR_RO & 0x3;
          }
          else
          {
            u32int apBits = mapAccessPermissionBits(((newSd->ap2 << 2) | newSd->ap10), newSd->domain);
            shadowSd->ap2  = (apBits >> 2) & 0x1;
            shadowSd->ap10 =  apBits & 0x3;
          }
        } // if AP bits have been changed
      } // edit details of: SECTION
      else if (oldGuestEntry->type == PAGE_TABLE)
      {
        pageTableDescriptor* newPtd = (pageTableDescriptor*)newGuestEntry;
        pageTableDescriptor* oldPtd = (pageTableDescriptor*)oldGuestEntry;
        if(oldPtd->addr != newPtd->addr)
        {
          //Change in address is same as a removeEntry and addNew one
          removePageTableEntry((pageTableDescriptor*)shadowEntry);
          copyPageTableEntry(newPtd, (pageTableDescriptor*)shadowEntry);
        }
        // now changing misc PTE details
        DIE_NOW(gc, "pageTableEdit: edit misc page_table PTE details case unimplemented.");
      }
      else if (oldGuestEntry->type == FAULT)
      {
        // changing details of a fault entry. wtf?
        DIE_NOW(gc, "pageTableEdit: edit fault PTE details case. wtf.");
      }
    }
    else
    {
      // second level entry
      if (oldGuestEntry->type == LARGE_PAGE)
      {
        DIE_NOW(0, "pageTableEdit: edit large page case unimplemented");
      }
      else if (oldGuestEntry->type == SMALL_PAGE)
      {
        DIE_NOW(0, "pageTableEdit: edit small page case unimplemented");
      }
      else if (oldGuestEntry->type == FAULT)
      {
        DIE_NOW(0, "pageTableEdit: edit 2nd lvl fault case. wtf?");
      }
    }
    
  }

  clearTLBbyMVA(address);
}

// Given a virtual address, retrieves the underlying physical address
u32int getPhysicalAddress(descriptor* ptd, u32int virtualAddress)
{
  descriptor* pte1st = get1stLevelPtDescriptorAddr(ptd, virtualAddress);
  switch(pte1st->type)
  {
    case SECTION:
    {
      sectionDescriptor* sd = (sectionDescriptor*)pte1st;
      if(1 == sd->sectionType)
      {
        DIE_NOW(0, "UNIMPLEMENTED: supersection getPhysicalAddress (pageTable.c)");
      }
      else
      {
        u32int result = (sd->addr << 20) | (virtualAddress & 0x000FFFFF);
        return result;
      }
      break;
    }
    case PAGE_TABLE:
    {
      descriptor* pte2nd = get2ndLevelPtDescriptor((pageTableDescriptor*)pte1st, virtualAddress);
      switch(pte2nd->type)
      {
        case LARGE_PAGE:
        {
          largeDescriptor* ld = (largeDescriptor*)pte2nd;
          return (ld->addr << 16) | (virtualAddress & 0x0000FFFF);
          break;
        }
        case SMALL_PAGE: //fall through
        case SMALL_PAGE_3:
        {
          smallDescriptor* sd = (smallDescriptor*)pte2nd;
          return (sd->addr << 12) | (virtualAddress & 0x00000FFF);
          break;
        }
        case FAULT: //fall through
        default:
          return 0xffffffff; //most likly to be invalid / cause an error!
          break;
      }
      break;
    }
    case FAULT: //fall through
    case RESERVED: //fall through
    default:
      return 0xffffffff;
      break;
  }
}

void disableHypervisorCaches(descriptor* ptd)
{
  sectionDescriptor* sd;

  u32int i;
  const u32int hypervisorEndAddr = (HYPERVISOR_START_ADDR + (TOTAL_MACHINE_RAM/4)) >> 20;
  for(i= (HYPERVISOR_START_ADDR >> 20); i <= hypervisorEndAddr; i++ )
  {
    sd = (sectionDescriptor*)(&ptd[i]);
    sd->c=0;
    sd->b=0;
    sd->tex=0;
  }
}

/** Set of asserts used to check constraints on the pageTable subsystem at compile time */
void pt_compile_time_check(void)
{
  /* Check packed struct size are correct */
  COMPILE_TIME_ASSERT((sizeof(descriptor) == sizeof(u32int)) , _descriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(pageTableDescriptor) == sizeof(u32int)) , _pageTableDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(sectionDescriptor) == sizeof(u32int)) , _sectionDescriptor_struct_not_32bit);
  //COMPILE_TIME_ASSERT((sizeof(superDescriptor) == sizeof(u32int)) , _superDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(largeDescriptor) == sizeof(u32int)) , _largeDescriptor_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(smallDescriptor) == sizeof(u32int)) , _smallDescriptor_struct_not_32bit);

  /* Check the pageType enum */
  COMPILE_TIME_ASSERT(0 == FAULT, __pageType_fault_enum_has_changed_);
  COMPILE_TIME_ASSERT(1 == PAGE_TABLE, __pageType_page_table_enum_has_changed_);
  COMPILE_TIME_ASSERT(2 == SECTION, __pageType_section_enum_has_changed_);
  COMPILE_TIME_ASSERT(3 == RESERVED, __pageType_reserved_enum_has_changed_);
  COMPILE_TIME_ASSERT(1 == LARGE_PAGE, __pageType_large_page_enum_has_changed_);
  COMPILE_TIME_ASSERT(2 == SMALL_PAGE, __pageType_small_page_enum_has_changed_);
  COMPILE_TIME_ASSERT(3 == SMALL_PAGE_3, __pageType_small_page_3_enum_has_changed_);
}

/**
 * simple method, just to clear up our guestSecondLvlPageTables array
 * called when PT1 is changed - new base page table data...
 **/
void removePT2Metadata(void)
{
  // zero metadata array
  int i = 0;
  for (i = 0; i < 256; i++)
  {
    guestSecondLvlPageTables[i] = 0;
  }
}

/**
 * this function invalidates all entries in a given shadow 1st level page table
 * frees up used malloc memory // TODO
 **/ 
void invalidateSPT1(descriptor* spt)
{
  u32int i = 0;
  for (i = 0; i < PAGE_TABLE_ENTRIES; i++)
  {  
    spt[i].type = FAULT;
  }
}

u32int mapAccessPermissionBits(u32int guestAP, u32int guestDomain)
{
  GCONTXT* context = getGuestContext();
  u32int shadowAPbits = 0;
  
  u32int dacr = getCregVal(3, 0, 0, 0, &context->coprocRegBank[0]);
  u32int domBits = (dacr >> guestDomain) & 0x3;

  switch (domBits)
  {
    case 0:
    {
      // no access. AP bits priv R/W user N/A
      shadowAPbits = PRIV_RW_USR_NO;
      break;
    }
    case 1:
    {
      // client access. actually map guest to shadow AP bits
      switch(guestAP)
      {
        case PRIV_NO_USR_NO:
          shadowAPbits = PRIV_RW_USR_NO;
          break;
        case PRIV_RW_USR_NO:
          shadowAPbits = PRIV_RW_USR_NO;
          break;
        case PRIV_RW_USR_RO:
          shadowAPbits = PRIV_RW_USR_RO;
          break;
        case PRIV_RW_USR_RW:
          shadowAPbits = PRIV_RW_USR_RW;
          break;
        case PRIV_RO_USR_NO:
          shadowAPbits = PRIV_RW_USR_NO;
          break;
        case DEPRECATED:
          shadowAPbits = PRIV_RW_USR_RO;
          break;
        case PRIV_RO_USR_RO:
          shadowAPbits = PRIV_RW_USR_RO;
          break;
        case AP_RESERVED:
        default:
          DIE_NOW(context, "Invalid PT entry access permission bits.");
      }
      break;
    }
    case 2:
    {
      DIE_NOW(context, "mapAccessPermissionBits: domain - reserved.");
    }
    case 3:
    {
      // manager access. allow all guest access
      shadowAPbits = PRIV_RW_USR_RW;
      break;
    }
  } // domain switch ends

  return shadowAPbits;
}

void removePageTableEntry(pageTableDescriptor* shadow)
{
  DIE_NOW(0, "UNIMPLEMENTED: removePageTableEntry");
}

void removeSectionEntry(sectionDescriptor* shadow)
{
  // Need to flush block cache at these addresses first
  // but which addresses to flush? shadow entries might have been fragmented to pages from a section...
  // so flush address range that the guest mapped originally.
  GCONTXT* context = getGuestContext();
  u32int vAddr = (((u32int)shadow - (u32int)context->PT_shadow) / 4) * 1024 * 1024;

  // would gladly remove this entry, but must check if the guest didnt decide
  // to remove pte that we used for the hypervisor
  u32int startAddr = HYPERVISOR_START_ADDR;
  u32int endAddr = startAddr + TOTAL_MACHINE_RAM/4;
  if ((startAddr <= vAddr) && (vAddr <= endAddr))
  {
    DIE_NOW(context, "Guest trying to unmap a VA that the hypervisor lives in");
  }
  else
  {
    validateCacheMultiPreChange(context->blockCache, vAddr, (vAddr+SECTION_SIZE-1));
    *(u32int*)shadow = 0;
  }
}

void removeLargePageEntry(largeDescriptor* shadow)
{
  DIE_NOW(0, "UNIMPLEMENTED: removeLargePageEntry");
}

void removeSmallPageEntry(smallDescriptor* shadow)
{
  DIE_NOW(0, "UNIMPLEMENTED: removeSmallPageEntry");
}

