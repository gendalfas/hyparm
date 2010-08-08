#include "timer32k.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "dataMoveInstr.h"

extern GCONTXT * getGuestContext(void);

static u32int timer32SysconfReg = 0;

u32int loadTimer32k(device * dev, ACCESS_SIZE size, u32int address)
{
  u32int val = 0;

  //We care about the real physical address of the entry, not its virtual address
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef TIMER32K_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from physical address: 0x");
  serial_putint(phyAddr);
  serial_putstring(", virtual address: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_newline();
#endif

  if (size == WORD)
  {
    u32int regAddr = phyAddr - TIMER_32K; 
    if (regAddr == REG_TIMER_32K_SYSCONFIG)
    {
      val = timer32SysconfReg;
    }
    else if (regAddr == REG_TIMER_32K_COUNTER)
    {
      // for now, just load the real counter value.
      volatile u32int * memPtr = (u32int*)address;
      val = *memPtr;
      serial_putstring(dev->deviceName);
      serial_putstring(" load counter value ");
      serial_putint(val);
      serial_newline();
    }
    else
    {
      serial_putstring(dev->deviceName);
      serial_putstring(" load from physical address: 0x");
      serial_putint(phyAddr);
      serial_putstring(", virtual address: 0x");
      serial_putint(address);
      serial_ERROR(" invalid register!");
    }
  }
  else
  {
    serial_putstring(dev->deviceName);
    serial_putstring(" load from physical address: 0x");
    serial_putint(phyAddr);
    serial_putstring(", virtual address: 0x");
    serial_putint(address);
    serial_ERROR(" invalid register access size (non32bit)");
  }
  return val;
}

void storeTimer32k(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  serial_ERROR("32k timer store unimplemented.");
}

void initTimer32k()
{
  // sysconf value is emulated. Reset to zero
  timer32SysconfReg = 0;
}