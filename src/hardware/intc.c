#include "serial.h"
#include "intc.h"
#include "memFunctions.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "beIntc.h"

extern GCONTXT * getGuestContext(void);

struct InterruptController * irqController;

void initIntc(void)
{
  irqController = (struct InterruptController*)mallocBytes(sizeof(struct InterruptController));
  if (irqController == 0)
  {
    serial_ERROR("Failed to allocate INTC.");
  }
  else
  {
    memset((void*)irqController, 0x0, sizeof(struct InterruptController));
#ifdef INTC_DBG
    serial_putstring("Initializing Interrupt controller at 0x");
    serial_putint((u32int)irqController);
    serial_newline();
#endif
  }
  intcReset();
}

/* top load function */
u32int loadIntc(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Intc: invalid access size.");
  }

  u32int regOffset = phyAddr - INTERRUPT_CONTROLLER;
  u32int val = 0;
  switch (regOffset)
  {
    case REG_INTCPS_REVISION:
      val = INTC_REVISION;
      break;
    case REG_INTCPS_SYSCONFIG:
      val = irqController->intcSysConfig;
      break;
    case REG_INTCPS_SYSSTATUS:
      val = irqController->intcSysStatus;
      // check reset done flag - it auto clears...
      if (irqController->intcSysStatus & INTCPS_SYSSTATUS_SOFTRESET)
      {
        irqController->intcSysStatus = irqController->intcSysStatus & ~INTCPS_SYSSTATUS_SOFTRESET;
      }
      break;
    case REG_INTCPS_MIR_CLEAR0:
      serial_ERROR("INTC: load from W/O register (MIR0_CLEAR)");
      break;
    case REG_INTCPS_MIR_CLEAR1:
      serial_ERROR("INTC: load from W/O register (MIR1_CLEAR)");
      break;
    case REG_INTCPS_MIR_CLEAR2:
      serial_ERROR("INTC: load from W/O register (MIR2_CLEAR)");
      break;
    case REG_INTCPS_MIR_SET0:
      serial_ERROR("INTC: load from W/O register (MIR0_SET)");
      break;
    case REG_INTCPS_MIR_SET1:
      serial_ERROR("INTC: load from W/O register (MIR1_SET)");
      break;
    case REG_INTCPS_MIR_SET2:
      serial_ERROR("INTC: load from W/O register (MIR2_SET)");
      break;
    case REG_INTCPS_ISR_CLEAR0:
      serial_ERROR("INTC: load from W/O register (ISR0_CLEAR)");
      break;
    case REG_INTCPS_ISR_CLEAR1:
      serial_ERROR("INTC: load from W/O register (ISR1_CLEAR)");
      break;
    case REG_INTCPS_ISR_CLEAR2:
      serial_ERROR("INTC: load from W/O register (ISR2_CLEAR)");
      break;
    case REG_INTCPS_PENDING_IRQ0:
      val = irqController->intcPendingIrq0;
#ifdef INTC_DBG
      serial_putstring("INTC: load pending irq0 value ");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case REG_INTCPS_PENDING_IRQ1:
      val = irqController->intcPendingIrq1;
#ifdef INTC_DBG
      serial_putstring("INTC: load pending irq1 value ");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case REG_INTCPS_PENDING_IRQ2:
      val = irqController->intcPendingIrq2;
#ifdef INTC_DBG
      serial_putstring("INTC: load pending irq2 value ");
      serial_putint(val);
      serial_newline();
#endif
      break;
    case REG_INTCPS_SIR_IRQ:
      val = prioritySortIrqs();
      break;
    case REG_INTCPS_CONTROL:
      val = irqController->intcControl & INTCPS_CONTROL_RESERVED;
      break;
    case REG_INTCPS_SIR_FIQ:
    case REG_INTCPS_PROTECTION:
    case REG_INTCPS_IDLE:
    case REG_INTCPS_IRQ_PRIORITY:
    case REG_INTCPS_FIQ_PRIORITY:
    case REG_INTCPS_THRESHOLD:
    case REG_INTCPS_ITR0:
    case REG_INTCPS_ITR1:
    case REG_INTCPS_ITR2:
    case REG_INTCPS_ISR_SET0:
    case REG_INTCPS_ISR_SET1:
    case REG_INTCPS_ISR_SET2:
    case REG_INTCPS_PENDING_FIQ0:
    case REG_INTCPS_PENDING_FIQ1:
    case REG_INTCPS_PENDING_FIQ2:
    case REG_INTCPS_ILR0:
    case REG_INTCPS_ILR1:
    case REG_INTCPS_ILR2:
    case REG_INTCPS_ILR3:
    case REG_INTCPS_ILR4:
    case REG_INTCPS_ILR5:
    case REG_INTCPS_ILR6:
    case REG_INTCPS_ILR7:
    case REG_INTCPS_ILR8:
    case REG_INTCPS_ILR9:
    case REG_INTCPS_ILR10:
    case REG_INTCPS_ILR11:
    case REG_INTCPS_ILR12:
    case REG_INTCPS_ILR13:
    case REG_INTCPS_ILR14:
    case REG_INTCPS_ILR15:
    case REG_INTCPS_ILR16:
    case REG_INTCPS_ILR17:
    case REG_INTCPS_ILR18:
    case REG_INTCPS_ILR19:
    case REG_INTCPS_ILR20:
    case REG_INTCPS_ILR21:
    case REG_INTCPS_ILR22:
    case REG_INTCPS_ILR23:
    case REG_INTCPS_ILR24:
    case REG_INTCPS_ILR25:
    case REG_INTCPS_ILR26:
    case REG_INTCPS_ILR27:
    case REG_INTCPS_ILR28:
    case REG_INTCPS_ILR29:
    case REG_INTCPS_ILR30:
    case REG_INTCPS_ILR31:
    case REG_INTCPS_ILR32:
    case REG_INTCPS_ILR33:
    case REG_INTCPS_ILR34:
    case REG_INTCPS_ILR35:
    case REG_INTCPS_ILR36:
    case REG_INTCPS_ILR37:
    case REG_INTCPS_ILR38:
    case REG_INTCPS_ILR39:
    case REG_INTCPS_ILR40:
    case REG_INTCPS_ILR42:
    case REG_INTCPS_ILR43:
    case REG_INTCPS_ILR44:
    case REG_INTCPS_ILR45:
    case REG_INTCPS_ILR47:
    case REG_INTCPS_ILR48:
    case REG_INTCPS_ILR49:
    case REG_INTCPS_ILR50:
    case REG_INTCPS_ILR51:
    case REG_INTCPS_ILR52:
    case REG_INTCPS_ILR53:
    case REG_INTCPS_ILR54:
    case REG_INTCPS_ILR56:
    case REG_INTCPS_ILR57:
    case REG_INTCPS_ILR58:
    case REG_INTCPS_ILR59:
    case REG_INTCPS_ILR60:
    case REG_INTCPS_ILR61:
    case REG_INTCPS_ILR62:
    case REG_INTCPS_ILR63:
    case REG_INTCPS_ILR64:
    case REG_INTCPS_ILR65:
    case REG_INTCPS_ILR66:
    case REG_INTCPS_ILR67:
    case REG_INTCPS_ILR68:
    case REG_INTCPS_ILR69:
    case REG_INTCPS_ILR70:
    case REG_INTCPS_ILR71:
    case REG_INTCPS_ILR72:
    case REG_INTCPS_ILR73:
    case REG_INTCPS_ILR75:
    case REG_INTCPS_ILR76:
    case REG_INTCPS_ILR77:
    case REG_INTCPS_ILR78:
    case REG_INTCPS_ILR79:
    case REG_INTCPS_ILR80:
    case REG_INTCPS_ILR81:
    case REG_INTCPS_ILR82:
    case REG_INTCPS_ILR83:
    case REG_INTCPS_ILR84:
    case REG_INTCPS_ILR85:
    case REG_INTCPS_ILR86:
    case REG_INTCPS_ILR87:
    case REG_INTCPS_ILR88:
    case REG_INTCPS_ILR89:
    case REG_INTCPS_ILR90:
    case REG_INTCPS_ILR91:
    case REG_INTCPS_ILR92:
    case REG_INTCPS_ILR93:
    case REG_INTCPS_ILR94:
    case REG_INTCPS_ILR95:
      serial_putstring("Intc: Unimplemted regsiter load.");
      serial_putstring("register number ");
      serial_putint(regOffset);
      serial_newline();
      serial_ERROR("PANIC");
      break;
    default:
      serial_ERROR("Intc: load on invalid register.");
  }
  
#ifdef INTC_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val = ");
  serial_putint(val);
  serial_newline();
#endif
  return val;
}


/* top store function */
void storeIntc(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef INTC_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Intc: invalid access size.");
  }

  u32int regOffset = phyAddr - INTERRUPT_CONTROLLER;
  switch (regOffset)
  {
    case REG_INTCPS_REVISION:
      serial_ERROR("Intc storing to read only register: version");
      break;
    case REG_INTCPS_SYSCONFIG:
      if (value & INTCPS_SYSCONFIG_SOFTRESET)
      {
#ifdef INTC_DBG
        serial_putstring("INTC: soft reset.");
        serial_newline();
#endif
        intcReset();
        value = value & ~INTCPS_SYSCONFIG_SOFTRESET;
      }
      // never set reset bit, all else except bit 0 is reserved
      irqController->intcSysConfig = value & INTCPS_SYSCONFIG_AUTOIDLE;
      break;
    case REG_INTCPS_SYSSTATUS:
      serial_ERROR("Intc storing to read only register: system status");
      break;
    case REG_INTCPS_SIR_IRQ:
      serial_ERROR("Intc storing to read only register: active irq");
      break;
    case REG_INTCPS_SIR_FIQ:
      serial_ERROR("Intc storing to read only register: active fiq");
      break;
    case REG_INTCPS_MIR_CLEAR0:
#ifdef INTC_DBG
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
          serial_putstring("INTC: clearing mask from interrupt number ");
          serial_putint_nozeros(i);
          serial_newline();
        }
      }
    }
#endif
      irqController->intcMir0 &= ~value;
      break;
    case REG_INTCPS_MIR_CLEAR1:
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
          // dedicate linux physical gpt2 for now, to serve as its core (gpt1)
          if (i+32 == GPT1_IRQ)
          {
            // linux unmasking gpt1 interrupt. unmask gpt2 in physical.
            unmaskInterruptBE(GPT2_IRQ);
          }
#ifdef INTC_DBG
          serial_putstring("INTC: clearing mask from interrupt number ");
          serial_putint_nozeros(i+32);
          serial_newline();
#endif
        }
      }
    }
      irqController->intcMir1 &= ~value;
      break;
    case REG_INTCPS_MIR_CLEAR2:
#ifdef INTC_DBG
    {
      u32int i;
      for (i = 0; i < 32; i++)
      {
        if (value & (1 << i))
        {
          serial_putstring("INTC: clearing mask from interrupt number ");
          serial_putint_nozeros(i+64);
          serial_newline();
        }
      }
    }
#endif
      irqController->intcMir2 &= ~value;
      break;
    case REG_INTCPS_MIR_SET0:
      irqController->intcMir0 |= value;
      irqController->intcPendingIrq0 = irqController->intcItr0 & ~irqController->intcMir0;
      break;
    case REG_INTCPS_MIR_SET1:
      irqController->intcMir1 |= value;
      irqController->intcPendingIrq1 = irqController->intcItr1 & ~irqController->intcMir1;
      break;
    case REG_INTCPS_MIR_SET2:
      irqController->intcMir2 |= value;
      irqController->intcPendingIrq2 = irqController->intcItr2 & ~irqController->intcMir2;
      break;
    case REG_INTCPS_CONTROL:
      irqController->intcControl = value & INTCPS_CONTROL_RESERVED;
      break;
    case REG_INTCPS_PROTECTION:
    case REG_INTCPS_IDLE:
    case REG_INTCPS_IRQ_PRIORITY:
    case REG_INTCPS_FIQ_PRIORITY:
    case REG_INTCPS_THRESHOLD:
    case REG_INTCPS_ITR0:
    case REG_INTCPS_ITR1:
    case REG_INTCPS_ITR2:
    case REG_INTCPS_MIR0:
    case REG_INTCPS_MIR1:
    case REG_INTCPS_MIR2:
    case REG_INTCPS_ISR_SET0:
    case REG_INTCPS_ISR_SET1:
    case REG_INTCPS_ISR_SET2:
    case REG_INTCPS_ISR_CLEAR0:
    case REG_INTCPS_ISR_CLEAR1:
    case REG_INTCPS_ISR_CLEAR2:
    case REG_INTCPS_PENDING_IRQ0:
    case REG_INTCPS_PENDING_IRQ1:
    case REG_INTCPS_PENDING_IRQ2:
    case REG_INTCPS_PENDING_FIQ0:
    case REG_INTCPS_PENDING_FIQ1:
    case REG_INTCPS_PENDING_FIQ2:
    case REG_INTCPS_ILR0:
    case REG_INTCPS_ILR1:
    case REG_INTCPS_ILR2:
    case REG_INTCPS_ILR3:
    case REG_INTCPS_ILR4:
    case REG_INTCPS_ILR5:
    case REG_INTCPS_ILR6:
    case REG_INTCPS_ILR7:
    case REG_INTCPS_ILR8:
    case REG_INTCPS_ILR9:
    case REG_INTCPS_ILR10:
    case REG_INTCPS_ILR11:
    case REG_INTCPS_ILR12:
    case REG_INTCPS_ILR13:
    case REG_INTCPS_ILR14:
    case REG_INTCPS_ILR15:
    case REG_INTCPS_ILR16:
    case REG_INTCPS_ILR17:
    case REG_INTCPS_ILR18:
    case REG_INTCPS_ILR19:
    case REG_INTCPS_ILR20:
    case REG_INTCPS_ILR21:
    case REG_INTCPS_ILR22:
    case REG_INTCPS_ILR23:
    case REG_INTCPS_ILR24:
    case REG_INTCPS_ILR25:
    case REG_INTCPS_ILR26:
    case REG_INTCPS_ILR27:
    case REG_INTCPS_ILR28:
    case REG_INTCPS_ILR29:
    case REG_INTCPS_ILR30:
    case REG_INTCPS_ILR31:
    case REG_INTCPS_ILR32:
    case REG_INTCPS_ILR33:
    case REG_INTCPS_ILR34:
    case REG_INTCPS_ILR35:
    case REG_INTCPS_ILR36:
    case REG_INTCPS_ILR37:
    case REG_INTCPS_ILR38:
    case REG_INTCPS_ILR39:
    case REG_INTCPS_ILR40:
    case REG_INTCPS_ILR42:
    case REG_INTCPS_ILR43:
    case REG_INTCPS_ILR44:
    case REG_INTCPS_ILR45:
    case REG_INTCPS_ILR47:
    case REG_INTCPS_ILR48:
    case REG_INTCPS_ILR49:
    case REG_INTCPS_ILR50:
    case REG_INTCPS_ILR51:
    case REG_INTCPS_ILR52:
    case REG_INTCPS_ILR53:
    case REG_INTCPS_ILR54:
    case REG_INTCPS_ILR56:
    case REG_INTCPS_ILR57:
    case REG_INTCPS_ILR58:
    case REG_INTCPS_ILR59:
    case REG_INTCPS_ILR60:
    case REG_INTCPS_ILR61:
    case REG_INTCPS_ILR62:
    case REG_INTCPS_ILR63:
    case REG_INTCPS_ILR64:
    case REG_INTCPS_ILR65:
    case REG_INTCPS_ILR66:
    case REG_INTCPS_ILR67:
    case REG_INTCPS_ILR68:
    case REG_INTCPS_ILR69:
    case REG_INTCPS_ILR70:
    case REG_INTCPS_ILR71:
    case REG_INTCPS_ILR72:
    case REG_INTCPS_ILR73:
    case REG_INTCPS_ILR75:
    case REG_INTCPS_ILR76:
    case REG_INTCPS_ILR77:
    case REG_INTCPS_ILR78:
    case REG_INTCPS_ILR79:
    case REG_INTCPS_ILR80:
    case REG_INTCPS_ILR81:
    case REG_INTCPS_ILR82:
    case REG_INTCPS_ILR83:
    case REG_INTCPS_ILR84:
    case REG_INTCPS_ILR85:
    case REG_INTCPS_ILR86:
    case REG_INTCPS_ILR87:
    case REG_INTCPS_ILR88:
    case REG_INTCPS_ILR89:
    case REG_INTCPS_ILR90:
    case REG_INTCPS_ILR91:
    case REG_INTCPS_ILR92:
    case REG_INTCPS_ILR93:
    case REG_INTCPS_ILR94:
    case REG_INTCPS_ILR95:
      serial_putstring("Intc: Unimplemted regsiter store.");
      serial_putstring("register number ");
      serial_putint(regOffset);
      serial_newline();
      serial_ERROR("PANIC");
      break;
    default:
      serial_ERROR("Intc: store on invalid register.");
  }
}

void intcReset()
{
  // reset all register values to defaults
  irqController->intcSysConfig   = 0x00000000;
  irqController->intcSysStatus   = 0x00000000;
  irqController->intcSirIrq      = 0xFFFFFFE0;
  irqController->intcSirFiq      = 0xFFFFFFE0;
  irqController->intcControl     = 0x00000000;
  irqController->intcProtection  = 0x00000000;
  irqController->intcIdle        = 0x00000000;
  irqController->intcIrqPriority = 0xFFFFFFC0;
  irqController->intcFiqPriority = 0xFFFFFFC0;
  irqController->intcThreshold   = 0x000000FF;
  irqController->intcItr0 = 0x0;
  irqController->intcItr1 = 0x0;
  irqController->intcItr2 = 0x0;
  irqController->intcMir0 = 0xFFFFFFFF;
  irqController->intcMir1 = 0xFFFFFFFF;
  irqController->intcMir2 = 0xFFFFFFFF;
  irqController->intcMirClear0 = 0x0;
  irqController->intcMirClear1 = 0x0;
  irqController->intcMirClear2 = 0x0;
  irqController->intcMirSet0 = 0x0;
  irqController->intcMirSet1 = 0x0;
  irqController->intcMirSet2 = 0x0;
  irqController->intcIsrSet0 = 0x0;
  irqController->intcIsrSet1 = 0x0;
  irqController->intcIsrSet2 = 0x0;
  irqController->intcIsrClear0 = 0x0;
  irqController->intcIsrClear1 = 0x0;
  irqController->intcIsrClear2 = 0x0;
  irqController->intcPendingIrq0 = 0x0;
  irqController->intcPendingIrq1 = 0x0;
  irqController->intcPendingIrq2 = 0x0;
  irqController->intcPendingFiq0 = 0x0;
  irqController->intcPendingFiq1 = 0x0;
  irqController->intcPendingFiq2 = 0x0;
  irqController->intcIlr0 = 0x0;
  irqController->intcIlr1 = 0x0;
  irqController->intcIlr2 = 0x0;
  irqController->intcIlr3 = 0x0;
  irqController->intcIlr4 = 0x0;
  irqController->intcIlr5 = 0x0;
  irqController->intcIlr6 = 0x0;
  irqController->intcIlr7 = 0x0;
  irqController->intcIlr8 = 0x0;
  irqController->intcIlr9 = 0x0;
  irqController->intcIlr10 = 0x0;
  irqController->intcIlr11 = 0x0;
  irqController->intcIlr12 = 0x0;
  irqController->intcIlr13 = 0x0;
  irqController->intcIlr14 = 0x0;
  irqController->intcIlr15 = 0x0;
  irqController->intcIlr16 = 0x0;
  irqController->intcIlr17 = 0x0;
  irqController->intcIlr18 = 0x0;
  irqController->intcIlr19 = 0x0;
  irqController->intcIlr20 = 0x0;
  irqController->intcIlr21 = 0x0;
  irqController->intcIlr22 = 0x0;
  irqController->intcIlr23 = 0x0;
  irqController->intcIlr24 = 0x0;
  irqController->intcIlr25 = 0x0;
  irqController->intcIlr26 = 0x0;
  irqController->intcIlr27 = 0x0;
  irqController->intcIlr28 = 0x0;
  irqController->intcIlr29 = 0x0;
  irqController->intcIlr30 = 0x0;
  irqController->intcIlr31 = 0x0;
  irqController->intcIlr32 = 0x0;
  irqController->intcIlr33 = 0x0;
  irqController->intcIlr34 = 0x0;
  irqController->intcIlr35 = 0x0;
  irqController->intcIlr36 = 0x0;
  irqController->intcIlr37 = 0x0;
  irqController->intcIlr38 = 0x0;
  irqController->intcIlr39 = 0x0;
  irqController->intcIlr40 = 0x0;
  irqController->intcIlr41 = 0x0;
  irqController->intcIlr42 = 0x0;
  irqController->intcIlr43 = 0x0;
  irqController->intcIlr44 = 0x0;
  irqController->intcIlr45 = 0x0;
  irqController->intcIlr46 = 0x0;
  irqController->intcIlr47 = 0x0;
  irqController->intcIlr48 = 0x0;
  irqController->intcIlr49 = 0x0;
  irqController->intcIlr50 = 0x0;
  irqController->intcIlr51 = 0x0;
  irqController->intcIlr52 = 0x0;
  irqController->intcIlr53 = 0x0;
  irqController->intcIlr54 = 0x0;
  irqController->intcIlr55 = 0x0;
  irqController->intcIlr56 = 0x0;
  irqController->intcIlr57 = 0x0;
  irqController->intcIlr58 = 0x0;
  irqController->intcIlr59 = 0x0;
  irqController->intcIlr60 = 0x0;
  irqController->intcIlr61 = 0x0;
  irqController->intcIlr62 = 0x0;
  irqController->intcIlr63 = 0x0;
  irqController->intcIlr64 = 0x0;
  irqController->intcIlr65 = 0x0;
  irqController->intcIlr66 = 0x0;
  irqController->intcIlr67 = 0x0;
  irqController->intcIlr68 = 0x0;
  irqController->intcIlr69 = 0x0;
  irqController->intcIlr70 = 0x0;
  irqController->intcIlr71 = 0x0;
  irqController->intcIlr72 = 0x0;
  irqController->intcIlr73 = 0x0;
  irqController->intcIlr74 = 0x0;
  irqController->intcIlr75 = 0x0;
  irqController->intcIlr76 = 0x0;
  irqController->intcIlr77 = 0x0;
  irqController->intcIlr78 = 0x0;
  irqController->intcIlr79 = 0x0;
  irqController->intcIlr80 = 0x0;
  irqController->intcIlr81 = 0x0;
  irqController->intcIlr82 = 0x0;
  irqController->intcIlr83 = 0x0;
  irqController->intcIlr84 = 0x0;
  irqController->intcIlr85 = 0x0;
  irqController->intcIlr86 = 0x0;
  irqController->intcIlr87 = 0x0;
  irqController->intcIlr88 = 0x0;
  irqController->intcIlr89 = 0x0;
  irqController->intcIlr90 = 0x0;
  irqController->intcIlr91 = 0x0;
  irqController->intcIlr92 = 0x0;
  irqController->intcIlr93 = 0x0;
  irqController->intcIlr94 = 0x0;
  irqController->intcIlr95 = 0x0;
  // reset done flag
  irqController->intcSysStatus = irqController->intcSysStatus | INTCPS_SYSSTATUS_SOFTRESET;
}

void maskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC: mask interrupt number out of range.");
  }
  
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);

  switch (bankNumber)
  {
    case 0:
      irqController->intcMir0 |= bitMask;
      break;  
    case 1:
      irqController->intcMir1 |= bitMask;
      break;  
    case 2:
      irqController->intcMir2 |= bitMask;
      break;  
    default:
      serial_ERROR("INTC: mask interrupt from invalid interrupt bank");
  }

}

void unmaskInterrupt(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC: unmask interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      irqController->intcMir0 &= ~bitMask;
      break;  
    case 1:
      irqController->intcMir1 &= ~bitMask;
      break;  
    case 2:
      irqController->intcMir2 &= ~bitMask;
      break;  
    default:
      serial_ERROR("INTC: unmask interrupt from invalid interrupt bank");
  }
}

bool isGuestIrqMasked(u32int interruptNumber)
{
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((interruptNumber < 0) || (interruptNumber >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC: isMasked interrupt number out of range.");
  }
  bankNumber = interruptNumber / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (interruptNumber % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      return ((irqController->intcMir0 & bitMask) == 1);
      break;  
    case 1:
      return ((irqController->intcMir1 & bitMask) == 1);
      break;  
    case 2:
      return ((irqController->intcMir2 & bitMask) == 1);
      break;  
    default:
      serial_ERROR("INTC: unmask interrupt from invalid interrupt bank");
  }
  // keep compiler quiet
  return TRUE;
}

u32int getIrqNumber(void)
{
  serial_ERROR("INTC: getIrqNumber - implement priority sorting of queued IRQS");
  return (irqController->intcSirIrq & INTCPS_SIR_IRQ_ACTIVEIRQ);
}

void setInterrupt(u32int irqNum)
{
  // 1. set raw interrupt signal before masking
  u32int bitMask = 0;
  u32int bankNumber = 0;

  if ((irqNum < 0) || (irqNum >= INTCPS_NR_OF_INTERRUPTS))
  {
    serial_ERROR("INTC: setInterrupt interrupt number out of range.");
  }
  bankNumber = irqNum / INTCPS_INTERRUPTS_PER_BANK;
  bitMask = 1 << (irqNum % INTCPS_INTERRUPTS_PER_BANK);
  switch (bankNumber)
  {
    case 0:
      irqController->intcItr0 |= bitMask;
      break;
    case 1:
      irqController->intcMir1 |= bitMask;
      break;
    case 2:
      irqController->intcMir2 |= bitMask;
      break;
    default:
      serial_ERROR("INTC: setInterrupt in invalid interrupt bank");
  }

  // 2. check mask, set signal after masking if needed.
  if(!isGuestIrqMasked(irqNum))
  {
    // unmasked! set flag...
    switch (bankNumber)
    {
      case 0:
        irqController->intcPendingIrq0 |= bitMask;
        break;
      case 1:
        irqController->intcPendingIrq1 |= bitMask;
        break;
      case 2:
        irqController->intcPendingIrq2 |= bitMask;
        break;
      default:
        serial_ERROR("INTC: setInterrupt in invalid interrupt bank");
    }
  }
  // 3. leave priority sorting for now. it will be done when IRQ number gets read.
}


// TODO: function to look through all pending irqs and select highest priority one
// return: interrupt number
u32int prioritySortIrqs()
{
  if ( (irqController->intcPendingIrq0 == 0) &&
       (irqController->intcPendingIrq1 == 0) &&
       (irqController->intcPendingIrq2 == 0) )
  {
    // no interrupts pending.
    return 0;
  }
  else
  {
    // for now just one interrupt delivered ot the guest! no priority sorting needed. 
    return GPT1_IRQ;
  }
}


bool isIrqPending()
{
  return ( (irqController->intcPendingIrq0 != 0) ||
           (irqController->intcPendingIrq1 != 0) ||
           (irqController->intcPendingIrq2 != 0) );
}

void intcDumpRegisters(void)
{
  serial_putstring("INTC: Revision ");
  serial_putint(INTC_REVISION);
  serial_newline();

  serial_putstring("INTC: sysconfig reg ");
  serial_putint(irqController->intcSysConfig);
  serial_newline();

  serial_putstring("INTC: sysStatus reg ");
  serial_putint(irqController->intcSysStatus);
  serial_newline();

  serial_putstring("INTC: current active irq reg ");
  serial_putint(irqController->intcSirIrq);
  serial_newline();

  serial_putstring("INTC: current active fiq reg ");
  serial_putint(irqController->intcSirFiq);
  serial_newline();

  serial_putstring("INTC: control reg ");
  serial_putint(irqController->intcControl);
  serial_newline();

  serial_putstring("INTC: protection reg ");
  serial_putint(irqController->intcProtection);
  serial_newline();

  serial_putstring("INTC: idle reg ");
  serial_putint(irqController->intcIdle);
  serial_newline();

  serial_putstring("INTC: current active irq priority ");
  serial_putint(irqController->intcIrqPriority);
  serial_newline();

  serial_putstring("INTC: current active fiq priority ");
  serial_putint(irqController->intcFiqPriority);
  serial_newline();

  serial_putstring("INTC: priority threshold ");
  serial_putint(irqController->intcThreshold);
  serial_newline();

  serial_putstring("INTC: interrupt status before masking:");
  serial_newline();
  serial_putint(irqController->intcItr0);
  serial_putint(irqController->intcItr1);
  serial_putint(irqController->intcItr2);
  serial_newline();

  serial_putstring("INTC: interrupt mask:");
  serial_newline();
  serial_putint(irqController->intcMir0);
  serial_putint(irqController->intcMir1);
  serial_putint(irqController->intcMir2);
  serial_newline();

  serial_putstring("INTC: pending IRQ:");
  serial_newline();
  serial_putint(irqController->intcPendingIrq0);
  serial_putint(irqController->intcPendingIrq1);
  serial_putint(irqController->intcPendingIrq2);
  serial_newline();

  serial_putstring("INTC: pending FIQ:");
  serial_newline();
  serial_putint(irqController->intcPendingFiq0);
  serial_putint(irqController->intcPendingFiq1);
  serial_putint(irqController->intcPendingFiq2);
  serial_newline();
}
