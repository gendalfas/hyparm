#include "gpmc.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Gpmc * gpmc;

void initGpmc()
{
  gpmc = (struct Gpmc*)mallocBytes(sizeof(struct Gpmc));
  if (gpmc == 0)
  {
    serial_ERROR("Failed to allocate GPMC.");
  }
  else
  {
    memset((void*)gpmc, 0x0, sizeof(struct Gpmc));
#ifdef GPMC_DBG
    serial_putstring("GPMC at 0x");
    serial_putint((u32int)gpmc);
    serial_newline();
#endif
  }
  
  // register default values
  gpmc->gpmcSysConfig = 0x00000010; // OMAP reference manual: 0x00000000
  gpmc->gpmcSysStatus = 0x00000001;
  gpmc->gpmcIrqStatus = 0x00000100; // OMAP reference manual: 0x00000000
  gpmc->gpmcIrqEnable = 0x00000000;
  gpmc->gpmcTimeoutControl = 0x00000000; // OMAP reference manual: 0x00001ff0
  gpmc->gpmcErrAddress = 0x00000000;
  gpmc->gpmcErrType = 0x00000000;
  gpmc->gpmcConfig = 0x00000000; // OMAP reference manual: 0x00000a00
  gpmc->gpmcStatus = 0x00000001;

  gpmc->gpmcConfig7_0 = 0x00000f00; // CS0 off; OMAP reference manual: 0x00000f40 (turns CS0 on)
  gpmc->gpmcConfig7_1 = 0x00000f00;
  gpmc->gpmcConfig7_2 = 0x00000f00;
  gpmc->gpmcConfig7_3 = 0x00000f00;
  gpmc->gpmcConfig7_4 = 0x00000f00;
  gpmc->gpmcConfig7_5 = 0x00000f00;
  gpmc->gpmcConfig7_6 = 0x00000f00;
  gpmc->gpmcConfig7_7 = 0x00000f00;

  gpmc->gpmcPrefetchConfig1 = 0x00000000; // OMAP reference manual: 0x00004000
  gpmc->gpmcPrefetchConfig2 = 0x00000000;
  gpmc->gpmcPrefetchControl = 0x00000000;
  gpmc->gpmcPrefetchStatus = 0x00000000;
  gpmc->gpmcEccConfig = 0x00000000; // OMAP reference manual: 0x00001030
  gpmc->gpmcEccControl = 0x00000000;
  gpmc->gpmcEccSizeConfig = 0x00000000; // OMAP reference manual: 0x3fcff000
  // TODO: add rest
}

/* load function */
u32int loadGpmc(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SYS_CTRL_MOD_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_newline();
#endif

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Gpmc: invalid access size.");
  }

  u32int regOffset = phyAddr - Q1_L3_GPMC;
  u32int val = 0;
  switch (regOffset)
  {
    case GPMC_REVISION:
      val = GPMC_REVISION_VALUE;
      break;
    case GPMC_SYSCONFIG:
      // TODO
      serial_putstring("WARN reading GPMC_SYSCONFIG ");
      serial_putint(gpmc->gpmcSysConfig);
      serial_newline();
      val = gpmc->gpmcSysConfig;
      break;
    case GPMC_NAND_COMMAND_0:
    case GPMC_NAND_ADDRESS_0:
    case GPMC_NAND_COMMAND_1:
    case GPMC_NAND_ADDRESS_1:
    case GPMC_NAND_COMMAND_2:
    case GPMC_NAND_ADDRESS_2:
    case GPMC_NAND_COMMAND_3:
    case GPMC_NAND_ADDRESS_3:
    case GPMC_NAND_COMMAND_4:
    case GPMC_NAND_ADDRESS_4:
    case GPMC_NAND_COMMAND_5:
    case GPMC_NAND_ADDRESS_5:
    case GPMC_NAND_COMMAND_6:
    case GPMC_NAND_ADDRESS_6:
    case GPMC_NAND_COMMAND_7:
    case GPMC_NAND_ADDRESS_7:
      serial_ERROR("Gpmc: load on write-only register.");
      break;
    default:
      serial_ERROR("Gpmc: load on invalid register.");
  }
  
  return val;
}


/* top store function */
void storeGpmc(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef GPMC_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" aSize ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif

  u32int regOffset = phyAddr - Q1_L3_GPMC;
  switch (regOffset)
  {
    case GPMC_SYSCONFIG:
      // TODO
      serial_putstring("WARN writing to GPMC_SYSCONFIG ");
      serial_putint(value);
      serial_newline();
      if (value & GPMC_SYSCONFIG_SOFTRESET)
      {
        serial_putstring("WARN should do soft reset of GPMC ");
      }
      gpmc->gpmcSysConfig = value & GPMC_SYSCONFIG_MASK;
      break;
    case GPMC_SYSSTATUS:
      serial_ERROR("Gpmc: store to read-only register.");
      break;
    default:
      serial_ERROR("Gpmc: store to invalid register.");
  }
}

