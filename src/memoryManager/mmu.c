#include "common/assert.h"
#include "common/debug.h"

#include "memoryManager/mmu.h"


static const char* dataAbtFaultString[] =
{
  "INVALID ENTRY",
  "Alignment Fault",
  "Debug Event",
  "Access Flag - Section",
  "Instruction Cache Maintenance Fault",
  "Translation Fault - Section",
  "Access Flag - Page",
  "Translation Fault - Page",
  "Synchronous External Abort",
  "Domain Fault - Section",
  "INVALID ENTRY",
  "Domain Fault - Page",
  "Translation Table Walk - 1st Level",
  "Permission Fault - Section",
  "Synchronous External Abort - 2nd Level",
  "Permission Fault - Page",
};

static const char* prefetchAbtFaultString[] =
{
  "INVALID ENTRY",                            //  0 = 0b00000,
  "INVALID ENTRY",                            //  1 = 0b00001,
  "Debug Event",                              //  2 = 0b00010,
  "Access Flag Fault: Section",               //  3 = 0b00011,
  "INVALID ENTRY",                            //  4 = 0b00100,
  "Translation Fault: Section",               //  5 = 0b00101,
  "Access Flag Fault: Page",                  //  6 = 0b00110,
  "Translation Fault: Page",                  //  7 = 0b00111,
  "Synchronous External Abort",               //  8 = 0b01000,
  "Domain Fault: Section",                    //  9 = 0b01001,
  "INVALID ENTRY",                            //  a = 0b01010,
  "Domain Fault: Page",                       //  b = 0b01011,
  "Translation Table Talk 1st Level sync abt",//  c = 0b01100,
  "Permission Fault: Section"                 //  d = 0b01101,
  "Translation Table Walk 2nd Level sync abt",//  e = 0b01110,
  "Permission Fault: Page",                   //  f = 0b01111,
  "INVALID ENTRY",                            // 10 = 0b10000,
  "INVALID ENTRY",                            // 11 = 0b10001,
  "INVALID ENTRY",                            // 12 = 0b10010,
  "INVALID ENTRY",                            // 13 = 0b10011,
  "IMPLEMENTATION DEFINED Lockdown",          // 14 = 0b10100,
  "INVALID ENTRY",                            // 15 = 0b10101,
  "INVALID ENTRY",                            // 16 = 0b10110,
  "INVALID ENTRY",                            // 17 = 0b10111,
  "INVALID ENTRY",                            // 18 = 0b11000,
  "Memory Access Synchronous Parity Error",   // 19 = 0b11001,
  "IMPLEMENTATION DEFINED Coprocessor Abort", // 1a = 0b11010,
  "INVALID ENTRY",                            // 1b = 0b11011,
  "Translation Table Walk 1st Level parity",  // 1c = 0b11100,
  "INVALID ENTRY",                            // 1d = 0b11101,
  "Translation Table Walk 2nd Level parity",  // 1e = 0b11110,
  "INVALID ENTRY",                            // 1f = 0b11111,
};

void clearTLB(void);
void clearCache(void);
void setTTBCR(u32int value);

void mmu_compile_time_check(void)
{
  COMPILE_TIME_ASSERT((sizeof(IFSR) == sizeof(u32int)) , _IFSR_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(DFSR) == sizeof(u32int)) , _DFSR_struct_not_32bit);
}

void mmuInit()
{
#ifdef MMU_DEBUG
  DEBUG_STRING("MMU init");
  DEBUG_NEWLINE();
#endif

  dataBarrier();
  clearTLB();
  clearCache();
  setTTBCR(0);
}

void dataBarrier()
{
#ifdef MMU_DEBUG
  DEBUG_STRING("Data Barrier");
  DEBUG_NEWLINE();
#endif
    //It doesn't matter which register it written/nor the value inside it
  asm ("mcr p15, 0, r0, c7, c10, 5"
  :
  :
  :"memory"
      );
}

void clearTLB()
{
#ifdef MMU_DEBUG
  DEBUG_STRING("Clearing TLB");
  DEBUG_NEWLINE();
#endif
  // mcr coproc opc1 Rt CRn CRm opc2
  //It doesn't matter which register it written/nor the value inside it
  asm ("mcr p15, 0, r0, c8, c7, 0"
  :
  :
  :"memory"
      );
}

//Clears any matching entries to <address> from the TLB
void clearTLBbyMVA(u32int address)
{
  address &= 0xFFFFF000;
#ifdef MMU_DEBUG
  DEBUG_STRING("Clearing TLB of virt addr: 0x");
  DEBUG_INT(address);
  DEBUG_NEWLINE();
#endif
  // mcr coproc opc1 Rt CRn CRm opc2
  asm ("mcr p15, 0, %0, c8, c7, 1"
  :
  :"r"(address)
  :"memory"
      );
}

void clearCache()
{
#ifdef MMU_DEBUG
  DEBUG_STRING("Clearing caches");
  DEBUG_NEWLINE();
#endif
  asm ("mcr p15, 0, r0, c7, c5, 0"
  :
  :
  :"memory"
      );
}

//Need to set range of TTBCR

void setTTBCR(u32int value)
{
  asm("mcr p15, 0, %0, c2, c0, 0"
  :
  :"r"(value)
  :"memory"
     );
}

void setDomain(u8int domain, access_type access)
{
  //access is a two bit field 00 = no access, 01=client, 10=reserved, 11=manager
#ifdef MMU_DEBUG
  DEBUG_STRING("Setting domain: ");
  DEBUG_INT_NOZEROS(domain);
  DEBUG_STRING(", with access bits 0x");
  DEBUG_INT_NOZEROS((u8int)access);
  DEBUG_NEWLINE();
#endif

  u32int value;
  asm volatile("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
     );
#ifdef MMU_DEBUG
  DEBUG_STRING("Domain Register before update: 0x");
  DEBUG_INT(value);
  DEBUG_NEWLINE();
#endif
  //clear the current domain
  u32int mask = ~(0b11 << (domain*2));
  value = value & mask;
  //Set the domain
  value = value | (access << ((domain)*2));
  asm volatile("mcr p15, 0, %0, c3, c0, 0"
  :
  : "r"(value)
  : "memory"
     );

#ifdef MMU_DEBUG
  asm volatile("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
              );
  DEBUG_STRING("Domain Register after update: 0x");
  DEBUG_INT(value);
  DEBUG_NEWLINE();
#endif

}


void setTexRemap(bool enable)
{
#ifdef MMU_DEBUG
  DEBUG_STRING("setTexRemap: ");
  DEBUG_INT(enable);
  DEBUG_NEWLINE();
#endif

  u32int value;
  asm volatile("mrc p15, 0, %0, c1, c0, 0"
  : "=r"(value)
  :
  : "memory"
     );
#ifdef MMU_DEBUG
  DEBUG_STRING("setTexRemap: currently SCTRL.TRE = ");
  u32int treEnabled = ((value & 0x10000000) == 0x10000000) ? TRUE : FALSE; 
  DEBUG_INT(treEnabled);
  DEBUG_NEWLINE();
#endif

  if (enable)
  {
    value |= 0x10000000;
  }
  else
  {
    value &= 0xEFFFFFFF;
  }

  asm volatile("mcr p15, 0, %0, c1, c0, 0"
  :
  : "r"(value)
  : "memory"
     );

#ifdef MMU_DEBUG
  asm volatile("mrc p15, 0, %0, c1, c0, 0"
  : "=r"(value)
  :
  : "memory"
              );
  DEBUG_STRING("setTexRemap: SCTRL after update ");
  DEBUG_INT(value);
  DEBUG_NEWLINE();
#endif
}


void mmuInsertPt0(descriptor* addr)
{
#ifdef MMU_DEBUG
  DEBUG_STRING("Add entry into TTBR0: 0x");
  DEBUG_INT((u32int)addr);
#endif
  //TODO: need to improve this to insert the correct bit masks
  asm("mcr p15, 0, %0,c2,c0,0"
  :
  :"r"(addr)
     );
}

void mmuInsertPt1(descriptor* addr)
{
#ifdef MMU_DEBUG
  DEBUG_STRING("Add entry into TTBR1: 0x");
  DEBUG_INT((u32int)addr);
#endif
  //TODO: need to improve this to insert the correct bit masks
  asm("mcr p15, 0, %0,c2,c0,1"
  :
  :"r"(addr)
     );
}

descriptor* mmuGetPt0()
{
  u32int regVal = 0;
  //TODO: need to improve this to insert the correct bit masks
  asm("mrc p15, 0, %0,c2,c0,0"
      :"=r"(regVal)
      :
      );
#ifdef MMU_DEBUG
  DEBUG_STRING("Get TTBR0: 0x");
  DEBUG_INT((u32int)regVal);
#endif
  return (descriptor*)regVal;
}



void mmuEnableVirtAddr()
{
  u32int tempReg;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t"
               "ORR %0, %0, #5\n\t" //enable MMU & Caching
               "mcr p15, 0, %0, c1, c0, 0\n\t"
               "mcr p15, 0, %0, c7, c5, 4\n\t" //ISB
  :"=r"(tempReg)
  :
  : "memory"
     );
}

void mmuDisableVirtAddr()
{
  u32int tempReg = 0;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t"
               "BIC %0, %0, #5\n\t" //enable MMU & Caching
               "mcr p15, 0, %0, c1, c0, 0\n\t"
  :"=r"(tempReg)
  :
  : "memory"
     );
}

bool isMmuEnabled()
{
  u32int tempReg = 0;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t"
  :"=r"(tempReg)
  :
  : "memory");
  
  return (tempReg & 0x1) ? TRUE : FALSE;
         
}

u32int getDFAR()
{
  u32int result;
  asm("mrc p15, 0, %0, c6, c0, 0"
  :"=r"(result)
     );
  return result;
}

DFSR getDFSR()
{
  DFSR result;
  asm("mrc p15, 0, %0, c5, c0, 0"
  :"=r"(result)
     );
  return result;
}

u32int getIFAR()
{
  u32int result;
  asm("mrc p15, 0, %0, c6, c0, 2"
  :"=r"(result)
     );
  return result;
}

IFSR getIFSR()
{
  IFSR result;
  asm("mrc p15, 0, %0, c5, c0, 1"
  :"=r"(result)
     );
  return result;
}

void printDataAbort()
{
  DFSR dfsr = getDFSR();
  u32int dfar = getDFAR();

  DEBUG_STRING("Data Abort Address: 0x");
  DEBUG_INT(dfar);
  DEBUG_NEWLINE();

  DEBUG_STRING("Fault type: ");
  DEBUG_STRING((char*)dataAbtFaultString[dfsr.fs3_0]);
  DEBUG_STRING(" (0x");
  u32int faultStatus = dfsr.fs4 << 4 | dfsr.fs3_0;
  DEBUG_INT_NOZEROS(faultStatus);
  DEBUG_STRING(")");
  DEBUG_NEWLINE();

  DEBUG_STRING("domain: 0x");
  DEBUG_INT_NOZEROS(dfsr.domain);

  /* Perhaps read out the domain and spit out the permission bits set for that domain at this point? */

  DEBUG_STRING(", Write not Read: 0x");
  DEBUG_INT_NOZEROS(dfsr.WnR);

  DEBUG_STRING(", External: 0x");
  DEBUG_INT_NOZEROS(dfsr.ExT);
  DEBUG_NEWLINE();

}

void printPrefetchAbort()
{
  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();
  u32int faultStatus = ifsr.fs3_0 | (ifsr.fs4 << 4);
  DEBUG_STRING("Prefetch Abort Address: 0x");
  DEBUG_INT(ifar);
  DEBUG_NEWLINE();

  DEBUG_STRING("Fault type: ");
  DEBUG_STRING((char*)prefetchAbtFaultString[faultStatus]);
  DEBUG_STRING(" (0x");
  DEBUG_INT_NOZEROS(faultStatus);
  DEBUG_STRING(")");

  DEBUG_STRING("External: 0x");
  DEBUG_INT_NOZEROS(ifsr.ExT);
  DEBUG_NEWLINE();
}
