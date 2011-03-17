#include "miscInstructions.h"
#include "commonInstrFunctions.h"
#include "intc.h"
#include "debug.h"

u32int* nopPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "nop PCFunct unfinished\n");
  return 0;
}

u32int nopInstruction(GCONTXT * context)
{
  DIE_NOW(0, "nopInstruction is executed but not yet checked for blockCopyCompatibility");
  serial_putstring("ERROR: NOP instr ");
  serial_putint(context->endOfBlockInstr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_putstring(" should not have trapped!");
  serial_newline();
  dumpGuestContext(context);
  return 0;
}

u32int* bxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bx PCFunct unfinished\n");
  return 0;
}

u32int bxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bxInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
  
  u32int nextPC = 0;
  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  if (!evalCC(instrCC, cpsrCC))
  {
      nextPC = context->R15 + 4;
      return nextPC;
  }
  //check if switching to thumb mode
  u32int regDest = (instr & 0x0000000F);
  u32int addr = loadGuestGPR(regDest, context);
  if (addr & 0x1)
  {
    dumpGuestContext(context);
    DIE_NOW(0, "BX Rm switching to Thumb. Unimplemented\n");
  }
  nextPC = addr & 0xFFFFFFFE;
  return nextPC;
}

u32int* mulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mul PCFunct unfinished\n");
  return 0;
}

u32int mulInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mulInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "MUL unfinished\n");
}

u32int* mlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mla PCFunct unfinished\n");
  return 0;
}

u32int mlaInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mlaInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "MLA unfinished\n");
}

u32int* swpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "swp PCFunct unfinished\n");
  return 0;
}

u32int swpInstruction(GCONTXT * context)
{
  DIE_NOW(0, "swpInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SWP unfinished\n");
  return 0;
}

u32int* sumlalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sumlal PCFunct unfinished\n");
  return 0;
}

u32int sumlalInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sumlalInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SUMLAL unfinished\n");
  return 0;
}

u32int* sumullPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sumull PCFunct unfinished\n");
  return 0;
}

u32int sumullInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sumullInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SUMULL unfinished\n");
  return 0;
}

u32int* pliPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pli PCFunct unfinished\n");
  return 0;
}

u32int pliInstruction(GCONTXT * context)
{
  DIE_NOW(0, "pliInstruction is executed but not yet checked for blockCopyCompatibility");
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: PLI!");
  serial_newline();
#endif
  return context->R15+4;
}

u32int* dbgPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dbg PCFunct unfinished\n");
  return 0;
}

u32int dbgInstruction(GCONTXT * context)
{
  DIE_NOW(0, "dbgInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "DBG unfinished\n");
  return 0;
}

u32int* dmbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dmb PCFunct unfinished\n");
  return 0;
}

u32int dmbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "dmbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "DBM unfinished\n");
  return 0;
}

u32int* dsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "dsb PCFunct unfinished\n");
  return 0;
}

u32int dsbInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: DSB (ignored)!");
  serial_newline();
#endif
  return context->PCOfLastInstruction+4;
}

u32int* isbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "isb PCFunct shouldn't be used since isbInstructions are always emulated!!\n");
  return 0;
}

u32int isbInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: ISB (ignored)!");
  serial_newline();
#endif
  return context->PCOfLastInstruction+4;
}

u32int* bfcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bfc PCFunct unfinished\n");
  return 0;
}

u32int bfcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bfcInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "BFC unfinished\n");
  return 0;
}

u32int* bfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bfi PCFunct unfinished\n");
  return 0;
}

u32int bfiInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bfiInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "BFI unfinished\n");
  return 0;
}

u32int* mlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mls PCFunct unfinished\n");
  return 0;
}

u32int mlsInstruction(GCONTXT * context)
{
  DIE_NOW(0, "mlsInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "MLS unfinished\n");
  return 0;
}

u32int* movwPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{ //Can be optimized -> this instruction is always safe!

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=(*instructionAddr);

  return currBlockCopyCacheAddr;
}

u32int movwInstruction(GCONTXT * context)
{
  DIE_NOW(0, "movwInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "MOVW unfinished\n");
  return 0;
}

u32int* movtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "movt PCFunct unfinished\n");
  return 0;
}

u32int movtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "movtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "MOVT unfinished\n");
  return 0;
}

u32int* rbitPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rbit PCFunct unfinished\n");
  return 0;
}

u32int rbitInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rbitInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "RBIT unfinished\n");
  return 0;
}

u32int* usbfxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usbfx PCFunct unfinished\n");
  return 0;
}

u32int usbfxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "usbfxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USBFX unfinished\n");
  return 0;
}

u32int* smcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smc PCFunct unfinished\n");
  return 0;
}

u32int smcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smcInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMC unfinished\n");
  return 0;
}

u32int* clrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "clrex PCFunct unfinished\n");
  return 0;
}

u32int clrexInstruction(GCONTXT * context)
{
  DIE_NOW(0, "clrexInstruction is executed but not yet checked for blockCopyCompatibility");
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: CLREX!");
  serial_newline();
#endif
  return context->R15+4;
}

u32int* yieldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "yield PCFunct unfinished\n");
  return 0;
}

u32int yieldInstruction(GCONTXT * context)
{
  DIE_NOW(0, "yieldInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "YIELD unfinished\n");
  return 0;
}

u32int* wfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "wfe PCFunct unfinished\n");
  return 0;
}

u32int wfeInstruction(GCONTXT * context)
{
  DIE_NOW(0, "wfeInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "WFE unfinished\n");
  return 0;
}

u32int* wfiPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "wfi PCFunct unfinished\n");
  return 0;
}

u32int wfiInstruction(GCONTXT * context)
{
  DIE_NOW(0, "wfiInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "WFI unfinished\n");
  return 0;
}

u32int* sevPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sev PCFunct unfinished\n");
  return 0;
}

u32int sevInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sevInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SEV unfinished\n");
  return 0;
}

u32int* cpsiePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cpsie PCFunct unfinished\n");
  return 0;
}

u32int cpsieInstruction(GCONTXT * context)
{
  DIE_NOW(0, "cpsieInstruction is executed but not yet checked for blockCopyCompatibility");
  return cpsInstruction(context);
}

u32int* cpsidPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cpsid PCFunct unfinished\n");
  return 0;
}

u32int cpsidInstruction(GCONTXT * context)
{
  return cpsInstruction(context);
}

u32int* cpsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "cps PCFunct unfinished\n");
  return 0;
}

u32int cpsInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  u32int imod       = (instr & 0x000C0000) >> 18;
  u32int changeMode = (instr & 0x00020000) >> 17;
  u32int affectA    = (instr & 0x00000100) >>  8;
  u32int affectI    = (instr & 0x00000080) >>  7;
  u32int affectF    = (instr & 0x00000040) >>  6;
  u32int newMode    =  instr & 0x0000001F;
#ifdef ARM_INSTR_TRACE
  serial_putstring("CPS instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->PCOfLastInstruction);
  serial_newline();
#endif
  if ( ((imod == 0) && (changeMode == 0)) || (imod == 1) )
  {
    serial_putint(instr);
    DIE_NOW(0, ": CPS unpredictable case\n");
  }
  if (guestInPrivMode(context))
  {
    u32int oldCpsr = context->CPSR;
    if (imod == 0x2) // enable
    {
#ifdef ARM_INSTR_TRACE
      serial_putstring("IMod: enable case");
      serial_newline();
#endif
      if (affectA != 0)
      {
        if ((oldCpsr & CPSR_AAB_BIT) != 0)
        {
          DIE_NOW(0, "Guest enabling async aborts globally!");
        }
        oldCpsr &= ~CPSR_AAB_BIT;
      }
      if (affectI)
      {
        if ( (oldCpsr & CPSR_IRQ_BIT) != 0)
        {
#ifdef ARM_INSTR_TRACE
          serial_putstring("Guest enabling irqs globally!");
          serial_newline();
#endif
          // chech interrupt controller if there is an interrupt pending
          if(isIrqPending())
          {
            context->guestIrqPending = TRUE;
          }
        } 
        oldCpsr &= ~CPSR_IRQ_BIT;
      }
      if (affectF)
      {
        if ( (oldCpsr & CPSR_FIQ_BIT) != 0)
        {
          DIE_NOW(0, "Guest enabling fiqs globally!");
        } 
        oldCpsr &= ~CPSR_FIQ_BIT;
      }
    }
    else if (imod == 3) // disable
    {
      if (affectA)
      {
        if ((oldCpsr & CPSR_AAB_BIT) == 0)
        {
          DIE_NOW(0, "Guest disabling async aborts globally!");
        }
        oldCpsr |= CPSR_AAB_BIT;
      }
      if (affectI)
      {
        if ( (oldCpsr & CPSR_IRQ_BIT) == 0) // were enabled, now disabled
        {
          // chech interrupt controller if there is an interrupt pending
          if(context->guestIrqPending == TRUE)
          {
            context->guestIrqPending = FALSE;
          }
        } 
        oldCpsr |= CPSR_IRQ_BIT;
      }
      if (affectF)
      {
        if ( (oldCpsr & CPSR_FIQ_BIT) == 0)
        {
          DIE_NOW(0, "Guest disabling fiqs globally!");
        } 
        oldCpsr |= CPSR_FIQ_BIT;
      }
    }
    else
    {
      DIE_NOW(0, "CPS invalid IMOD\n");
    }
    // ARE we switching modes?
    if (changeMode)
    {
      oldCpsr &= ~CPSR_MODE_FIELD;
      oldCpsr |= newMode;
      DIE_NOW(0, "guest is changing execution modes. What?!");
    }
    context->CPSR = oldCpsr;
  }
  else
  {
    // guest is not in privileged mode! cps should behave as a nop, but lets see what went wrong.
    DIE_NOW(0, "CPS instruction: executed in guest user mode.");
  }
  return (context->PCOfLastInstruction+4);
}

u32int* pkhbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pkhbt PCFunct unfinished\n");
  return 0;
}

u32int pkhbtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "pkhbtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "PKHBT unfinished\n");
  return 0;
}

u32int* pkhtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pkhtb PCFunct unfinished\n");
  return 0;
}

u32int pkhtbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "pkhtbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "PKHTB unfinished\n");
  return 0;
}

u32int* qadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd16 PCFunct unfinished\n");
  return 0;
}

u32int qadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "qadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QADD16 unfinished\n");
  return 0;
}

u32int* qadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd8 PCFunct unfinished\n");
  return 0;
}

u32int qadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "qadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QADD8 unfinished\n");
  return 0;
}

u32int* qaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qaddsubx PCFunct unfinished\n");
  return 0;
}

u32int qaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qaddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QADDSUBX unfinished\n");
  return 0;
}

u32int* qsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub16 PCFunct unfinished\n");
  return 0;
}

u32int qsub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "qsub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QSUB16 unfinished\n");
  return 0;
}

u32int* qsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub8 PCFunct unfinished\n");
  return 0;
}

u32int qsub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "qsub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QSUB8 unfinished\n");
  return 0;
}

u32int* qsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsubaddx PCFunct unfinished\n");
  return 0;
}

u32int qsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qsubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QSUBADDX unfinished\n");
  return 0;
}

u32int* sadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sadd16 PCFunct unfinished\n");
  return 0;
}

u32int sadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "sadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SADD16 unfinished\n");
  return 0;
}

u32int* sadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sadd8 PCFunct unfinished\n");
  return 0;
}

u32int sadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "sadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SADD8 unfinished\n");
  return 0;
}

u32int* saddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "saddsubx PCFunct unfinished\n");
  return 0;
}

u32int saddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "saddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SADDADDX unfinished\n");
  return 0;
}

u32int* shadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shadd16 PCFunct unfinished\n");
  return 0;
}

u32int shadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "shadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHADD16 unfinished\n");
  return 0;
}

u32int* shadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shadd8 PCFunct unfinished\n");
  return 0;
}

u32int shadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "shadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHADD8 unfinished\n");
  return 0;
}

u32int* shaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shaddsubx PCFunct unfinished\n");
  return 0;
}

u32int shaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "shaddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHADDSUBX unfinished\n");
  return 0;
}

u32int* shsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsub16 PCFunct unfinished\n");
  return 0;
}

u32int shsub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "shsub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHSUB16 unfinished\n");
  return 0;
}

u32int* shsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsub8 PCFunct unfinished\n");
  return 0;
}

u32int shsub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "shsub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHSUB8 unfinished\n");
  return 0;
}

u32int* shsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "shsubaddx PCFunct unfinished\n");
  return 0;
}

u32int shsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "shsubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SHSUBADDX unfinished\n");
  return 0;
}

u32int* ssub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssub16 PCFunct unfinished\n");
  return 0;
}

u32int ssub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "ssub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SSUB16 unfinished\n");
  return 0;
}

u32int* ssub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssub8 PCFunct unfinished\n");
  return 0;
}

u32int ssub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "ssub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SSUB8 unfinished\n");
  return 0;
}

u32int* ssubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssubaddx PCFunct unfinished\n");
  return 0;
}

u32int ssubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ssubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SSUBADDX unfinished\n");
  return 0;
}

u32int* uadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uadd16 PCFunct unfinished\n");
  return 0;
}

u32int uadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UADD16 unfinished\n");
  return 0;
}

u32int* uadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uadd8 PCFunct unfinished\n");
  return 0;
}

u32int uadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UADD8 unfinished\n");
  return 0;
}

u32int* uaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uaddsubx PCFunct unfinished\n");
  return 0;
}

u32int uaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uaddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UADDSUBX unfinished\n");
  return 0;
}

u32int* uhadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhadd16 PCFunct unfinished\n");
  return 0;
}

u32int uhadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uhadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHADD16 unfinished\n");
  return 0;
}

u32int* uhadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhadd8 PCFunct unfinished\n");
  return 0;
}

u32int uhadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uhadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHADD8 unfinished\n");
  return 0;
}

u32int* uhaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhaddsubx PCFunct unfinished\n");
  return 0;
}

u32int uhaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uhaddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHADDSUBX unfinished\n");
  return 0;
}

u32int* uhsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsub16 PCFunct unfinished\n");
  return 0;
}

u32int uhsub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uhsub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHSUB16 unfinished\n");
  return 0;
}

u32int* uhsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsub8 PCFunct unfinished\n");
  return 0;
}

u32int uhsub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uhsub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHSUB8 unfinished\n");
  return 0;
}

u32int* uhsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uhsubaddx PCFunct unfinished\n");
  return 0;
}

u32int uhsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uhsubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UHSUBADDX unfinished\n");
  return 0;
}

u32int* uqadd16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqadd16 PCFunct unfinished\n");
  return 0;
}

u32int uqadd16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uqadd16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQADD16 unfinished\n");
  return 0;
}

u32int* uqadd8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqadd8 PCFunct unfinished\n");
  return 0;
}

u32int uqadd8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uqadd8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQADD8 unfinished\n");
  return 0;
}

u32int* uqaddsubxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqaddsubx PCFunct unfinished\n");
  return 0;
}

u32int uqaddsubxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uqaddsubxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQADDSUBX unfinished\n");
  return 0;
}

u32int* uqsub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsub16 PCFunct unfinished\n");
  return 0;
}

u32int uqsub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uqsub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQSUB16 unfinished\n");
  return 0;
}

u32int* uqsub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsub8 PCFunct unfinished\n");
  return 0;
}

u32int uqsub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uqsub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQSUB8 unfinished\n");
  return 0;
}

u32int* uqsubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uqsubaddx PCFunct unfinished\n");
  return 0;
}

u32int uqsubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uqsubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UQSUBADDX unfinished\n");
  return 0;
}

u32int* usub16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usub16 PCFunct unfinished\n");
  return 0;
}

u32int usub16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "usub16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USUB16 unfinished\n");
  return 0;
}

u32int* usub8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usub8 PCFunct unfinished\n");
  return 0;
}

u32int usub8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "usub8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USUB8 unfinished\n");
  return 0;
}

u32int* usubaddxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usubaddx PCFunct unfinished\n");
  return 0;
}

u32int usubaddxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "usubaddxInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USUBADDX unfinished\n");
  return 0;
}

u32int* revPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rev PCFunct unfinished\n");
  return 0;
}

u32int revInstruction(GCONTXT * context)
{
  DIE_NOW(0, "revInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "REV unfinished\n");
  return 0;
}

u32int* rev16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rev16 PCFunct unfinished\n");
  return 0;
}

u32int rev16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "rev16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "REV16 unfinished\n");
  return 0;
}

u32int* revshPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "revsh PCFunct unfinished\n");
  return 0;
}

u32int revshInstruction(GCONTXT * context)
{
  DIE_NOW(0, "revshInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "REVSH unfinished\n");
  return 0;
}

u32int* rfePCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "rfe PCFunct unfinished\n");
  return 0;
}

u32int rfeInstruction(GCONTXT * context)
{
  DIE_NOW(0, "rfeInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "RFE unfinished\n");
  return 0;
}

u32int* sxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxth PCFunct unfinished\n");
  return 0;
}

u32int sxthInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sxthInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTH unfinished\n");
  return 0;
}

u32int* sxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtb16 PCFunct unfinished\n");
  return 0;
}

u32int sxtb16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "sxtb16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTB16 unfinished\n");
  return 0;
}

u32int* sxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtb PCFunct unfinished\n");
  return 0;
}

u32int sxtbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sxtbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTB unfinished\n");
  return 0;
}

u32int* uxthPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxth PCFunct unfinished\n");
  return 0;
}

u32int uxthInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uxthInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTHunfinished\n");
  return 0;
}

u32int* uxtb16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtb16 PCFunct unfinished\n");
  return 0;
}

u32int uxtb16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uxtb16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTB16 unfinished\n");
  return 0;
}

u32int* uxtbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{//Can be optimized -> this instruction is always safe!
  u32int instruction = *instructionAddr;
  if( (instruction & 0xF) == 0xF)
  {
    DIE_NOW(0,"uxtbPCInstruction: Rm is PC -> unpredictable");
  }

  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int uxtbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uxtbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTB unfinished\n");
  return 0;
}

u32int* sxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtah PCFunct unfinished\n");
  return 0;
}

u32int sxtahInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sxtahInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTAH unfinished\n");
  return 0;
}

u32int* sxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtab16 PCFunct unfinished\n");
  return 0;
}

u32int sxtab16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "sxtab16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTAB16 unfinished\n");
  return 0;
}

u32int* sxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sxtab PCFunct unfinished\n");
  return 0;
}

u32int sxtabInstruction(GCONTXT * context)
{
  DIE_NOW(0, "sxtabInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SXTAB unfinished\n");
  return 0;
}

u32int* uxtahPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtah PCFunct unfinished\n");
  return 0;
}

u32int uxtahInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uxtahInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTAH unfinished\n");
  return 0;
}

u32int* uxtab16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtab16 PCFunct unfinished\n");
  return 0;
}

u32int uxtab16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "uxtab16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTAB16 unfinished\n");
  return 0;
}

u32int* uxtabPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "uxtab PCFunct unfinished\n");
  return 0;
}

u32int uxtabInstruction(GCONTXT * context)
{
  DIE_NOW(0, "uxtabInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UXTAB unfinished\n");
  return 0;
}

u32int* selPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "sel PCFunct unfinished\n");
  return 0;
}

u32int selInstruction(GCONTXT * context)
{
  DIE_NOW(0, "selInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SEL unfinished\n");
  return 0;
}

u32int* setendPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "setend PCFunct unfinished\n");
  return 0;
}

u32int setendInstruction(GCONTXT * context)
{
  DIE_NOW(0, "setendInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SETEND unfinished\n");
  return 0;
}

u32int* smuadPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smuad PCFunct unfinished\n");
  return 0;
}

u32int smuadInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smuadInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMUAD unfinished\n");
  return 0;
}

u32int* smusdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smusd PCFunct unfinished\n");
  return 0;
}

u32int smusdInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smusdInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMUSD unfinished\n");
  return 0;
}

u32int* smladPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlad PCFunct unfinished\n");
  return 0;
}

u32int smladInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smladInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLAD unfinished\n");
  return 0;
}

u32int* smlaldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlald PCFunct unfinished\n");
  return 0;
}

u32int smlaldInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlaldInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLALD unfinished\n");
  return 0;
}

u32int* smlsdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlsd PCFunct unfinished\n");
  return 0;
}

u32int smlsdInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlsdInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLSD unfinished\n");
  return 0;
}

u32int* smlsldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlsld PCFunct unfinished\n");
  return 0;
}

u32int smlsldInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlsldInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLSLD unfinished\n");
  return 0;
}

u32int* smmulPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmul PCFunct unfinished\n");
  return 0;
}

u32int smmulInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smmulInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMMUL unfinished\n");
  return 0;
}

u32int* smmlaPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmla PCFunct unfinished\n");
  return 0;
}

u32int smmlaInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smmlaInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMMLA unfinished\n");
  return 0;
}

u32int* smmlsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smmls PCFunct unfinished\n");
  return 0;
}

u32int smmlsInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smmlsInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMMLS unfinished\n");
  return 0;
}

u32int* srsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "srs PCFunct unfinished\n");
  return 0;
}

u32int srsInstruction(GCONTXT * context)
{
  DIE_NOW(0, "srsInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SRS unfinished\n");
  return 0;
}

u32int* ssatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssat PCFunct unfinished\n");
  return 0;
}

u32int ssatInstruction(GCONTXT * context)
{
  DIE_NOW(0, "ssatInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SSAT unfinished\n");
  return 0;
}

u32int* ssat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "ssat16 PCFunct unfinished\n");
  return 0;
}

u32int ssat16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "ssat16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "ssat16 unfinished\n");
  return 0;
}

u32int* umaalPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "umaal PCFunct unfinished\n");
  return 0;
}

u32int umaalInstruction(GCONTXT * context)
{
  DIE_NOW(0, "umaalInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "UMAAL unfinished\n");
  return 0;
}

u32int* usad8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usad8 PCFunct unfinished\n");
  return 0;
}

u32int usad8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "usad8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USAD8 unfinished\n");
  return 0;
}

u32int* usada8PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usada8 PCFunct unfinished\n");
  return 0;
}

u32int usada8Instruction(GCONTXT * context)
{
  DIE_NOW(0, "usada8Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USADA unfinished\n");
  return 0;
}

u32int* usatPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usat PCFunct unfinished\n");
  return 0;
}

u32int usatInstruction(GCONTXT * context)
{
  DIE_NOW(0, "usatInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USAT unfinished\n");
  return 0;
}

u32int* usat16PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "usat16 PCFunct unfinished\n");
  return 0;
}

u32int usat16Instruction(GCONTXT * context)
{
  DIE_NOW(0, "usat16Instruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "USAT16 unfinished\n");
  return 0;
}

u32int* bxjPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bxj PCFunct unfinished\n");
  return 0;
}

u32int bxjInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bxjInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "BXJ unfinished\n");
  return 0;
}

u32int* bkptPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "bkpt PCFunct unfinished\n");
  return 0;
}

u32int bkptInstruction(GCONTXT * context)
{
  DIE_NOW(0, "bkptInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "BKPT unfinished\n");
  return 0;
}

u32int* blxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "blx PCFunct unfinished\n");
  return 0;
}

u32int blxInstruction(GCONTXT * context)
{
  DIE_NOW(0, "blxInstruction is executed but not yet checked for blockCopyCompatibility");
  u32int instr = context->endOfBlockInstr;
  if ((instr & 0xfe000000) == 0xfa000000)
  {
    dumpGuestContext(context);
    DIE_NOW(0, "BLX #imm24 switching to Thumb. Unimplemented.\n");
  }
  u32int nextPC = 0;
  
  u32int instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC  = (context->CPSR >> 28) & 0xF;
  if (!evalCC(instrCC, cpsrCC))
  {
    nextPC = context->R15 + 4;
    return nextPC;
  }
  u32int regDest = (instr & 0x0000000F); // holds dest addr and mode bit
  u32int value = loadGuestGPR(regDest, context);
  u32int thumbMode = value & 0x1;
  if (thumbMode)
  {
    dumpGuestContext(context);
    DIE_NOW(0, "BLX Rm switching to Thumb. Unimplemented.\n");
  }
  // link register
  storeGuestGPR(14, context->R15+4, context);
  nextPC = value & 0xFFFFFFFE;
  return nextPC;
}

u32int* clzPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  u32int instruction = *instructionAddr;

  if((instruction & 0xF) == 0xF || ((instruction>>12 & 0xF)==0xF))
  {  //see ARM ARM clz p.384
    DIE_NOW(0, "clz PCFunct: bits 0-3 = 0xF -> UNPREDICTABLE BEHAVIOR\n");
  }
  //Since no register can be PC the instruction is save to execute -> copy it to blockCopyCache
  currBlockCopyCacheAddr=checkAndClearBlockCopyCacheAddress(currBlockCopyCacheAddr,context->blockCache,(u32int*)context->blockCopyCache,(u32int*)context->blockCopyCacheEnd);
  *(currBlockCopyCacheAddr++)=instruction;

  return currBlockCopyCacheAddr;
}

u32int clzInstruction(GCONTXT * context)
{
  DIE_NOW(0, "clzInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "CLZ unfinished\n");
  return 0;
}

u32int* pldPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "pld PCFunct unfinished\n");
  return 0;
}

u32int pldInstruction(GCONTXT * context)
{
#ifdef ARM_INSTR_TRACE
  serial_putstring("Warning: PLD!");
  serial_newline();
#endif
  return context->PCOfLastInstruction+4;
}

u32int* smlabbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlabb PCFunct unfinished\n");
  return 0;
}

u32int smlabbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlabbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLABB unfinished\n");
  return 0;
}

u32int* smlatbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlatb PCFunct unfinished\n");
  return 0;
}

u32int smlatbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlatbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLATB unfinished\n");
  return 0;
}

u32int* smlabtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlabt PCFunct unfinished\n");
  return 0;
}

u32int smlabtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlabtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLABT unfinished\n");
  return 0;
}

u32int* smlattPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlatt PCFunct unfinished\n");
  return 0;
}

u32int smlattInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlattInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLATT unfinished\n");
  return 0;
}

u32int* smlawbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlawb PCFunct unfinished\n");
  return 0;
}

u32int smlawbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlawbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLAWB unfinished\n");
  return 0;
}

u32int* smlawtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlawt PCFunct unfinished\n");
  return 0;
}

u32int smlawtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlawtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLAWT unfinished\n");
  return 0;
}

u32int* smlalbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlalbb PCFunct unfinished\n");
  return 0;
}

u32int smlalbbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlalbbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLALBB unfinished\n");
  return 0;
}

u32int* smlaltbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlaltb PCFunct unfinished\n");
  return 0;
}

u32int smlaltbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlaltbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLALTB unfinished\n");
  return 0;
}

u32int* smlalbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlalbt PCFunct unfinished\n");
  return 0;
}

u32int smlalbtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlalbtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLALBT unfinished\n");
  return 0;
}

u32int* smlalttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smlaltt PCFunct unfinished\n");
  return 0;
}

u32int smlalttInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smlalttInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMLALTT unfinished\n");
  return 0;
}

u32int* smulbbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulbb PCFunct unfinished\n");
  return 0;
}

u32int smulbbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smulbbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULBB unfinished\n");
  return 0;
}

u32int* smultbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smultb PCFunct unfinished\n");
  return 0;
}

u32int smultbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smultbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULTB unfinished\n");
  return 0;
}

u32int* smulbtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulbt PCFunct unfinished\n");
  return 0;
}

u32int smulbtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smulbtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULBT unfinished\n");
  return 0;
}

u32int* smulttPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smultt PCFunct unfinished\n");
  return 0;
}

u32int smulttInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smulttInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULTT unfinished\n");
  return 0;
}

u32int* smulwbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulwb PCFunct unfinished\n");
  return 0;
}

u32int smulwbInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smulwbInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULWD unfinished\n");
  return 0;
}

u32int* smulwtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "smulwt PCFunct unfinished\n");
  return 0;
}

u32int smulwtInstruction(GCONTXT * context)
{
  DIE_NOW(0, "smulwtInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "SMULWT unfinished\n");
  return 0;
}

u32int* qaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qadd PCFunct unfinished\n");
  return 0;
}

u32int qaddInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qaddInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QADD unfinished\n");
  return 0;
}

u32int* qdaddPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qdadd PCFunct unfinished\n");
  return 0;
}

u32int qdaddInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qdaddInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QDADD unfinished\n");
  return 0;
}

u32int* qsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qsub PCFunct unfinished\n");
  return 0;
}

u32int qsubInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qsubInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QSUB unfinished\n");
  return 0;
}

u32int* qdsubPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "qdsub PCFunct unfinished\n");
  return 0;
}

u32int qdsubInstruction(GCONTXT * context)
{
  DIE_NOW(0, "qdsubInstruction is executed but not yet checked for blockCopyCompatibility");
  dumpGuestContext(context);
  DIE_NOW(0, "QDSUB unfinished\n");
  return 0;
}

u32int* msrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "msr PCFunct unfinished\n");
  return 0;
}

u32int msrInstruction(GCONTXT * context)
{
  //This instruction cannot read from PC so nothing special has to be done-> only change nextPC
  u32int instr = context->endOfBlockInstr;
  u32int instrCC =    (instr & 0xF0000000) >> 28;
  u32int regOrImm =   (instr & 0x02000000); // if 1 then imm12, 0 then Reg
  u32int cpsrOrSpsr = (instr & 0x00400000); // if 0 then cpsr, !0 then spsr
  u32int fieldMsk =   (instr & 0x000F0000) >> 16;
  
  u32int value = 0;
  u32int nextPC = 0;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  if (!evalCC(instrCC, cpsrCC))
  {
    nextPC = context->PCOfLastInstruction + 4;
    return nextPC;
  }
  
  if (regOrImm == 0)
  {
    // register case
    u32int regSrc = instr & 0x0000000F;
    if (regSrc == 0xF)
    {
      invalid_instruction(instr, "MSR cannot use PC as source register");
    }
    value = loadGuestGPR(regSrc, context);
  }
  else
  {
    // immediate case
    u32int immediate = instr & 0x00000FFF;
    value = armExpandImm12(immediate);
  }
    
  u32int oldValue = 0;
  if (cpsrOrSpsr == 0)
  {
    // CPSR!
    oldValue = context->CPSR;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR & CPSR_MODE_FIELD)
    {
      case CPSR_MODE_FIQ:
        oldValue = context->SPSR_FIQ;
        break;
      case CPSR_MODE_IRQ:
        oldValue = context->SPSR_IRQ;
        break;
      case CPSR_MODE_SVC:
        oldValue = context->SPSR_SVC;
        break;
      case CPSR_MODE_ABORT:
        oldValue = context->SPSR_ABT;
        break;
      case CPSR_MODE_UNDEF:
        oldValue = context->SPSR_UND;
        break;
      case CPSR_MODE_USER:
      case CPSR_MODE_SYSTEM:
      default: 
        DIE_NOW(0, "MSR: invalid SPSR write for current guest mode.");
    } 
  }
  // [3:0] field mask:
  // - bit 0: set control field (mode bits/interrupt bits)
  // - bit 1: set extension field (??? [15:8] of cpsr)
  // - bit 2: set status field (??? [23:16] of cpsr)
  // - bit 3: set condition flags of cpsr
  // control field [7-0] set.
  if ( ((fieldMsk & 0x1) == 0x1) && (guestInPrivMode(context)) )
  {
    // check for thumb toggle!
    if ((oldValue & CPSR_THUMB_BIT) != (value & CPSR_THUMB_BIT))
    {
      dumpGuestContext(context);
      DIE_NOW(0, "MSR toggle THUMB bit.");
    } 
    // is new CPSR - user mode?
    if ((value & CPSR_MODE_FIELD) == CPSR_MODE_USER)
    {
      DIE_NOW(context, "MSR switching CPSR to user mode\n");
    }
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0x000000FF);
    // clear old fields!
    oldValue &= 0xFFFFFF00;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x2) == 0x2) && (guestInPrivMode(context)) )
  {
    // extension field: async abt, endianness, IT[7:2]
    // check for async abt toggle!
    if ((oldValue & CPSR_AAB_BIT) != (value & CPSR_AAB_BIT))
    {
      dumpGuestContext(context);
      DIE_NOW(0, "MSR toggle async abort disable bit.");
    } 
    // check for endiannes toggle!
    if ((oldValue & CPSR_ENDIANNESS) != (value & CPSR_ENDIANNESS))
    {
      DIE_NOW(0, "MSR toggle endianess bit.");
    } 
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0x0000FF00);
    // clear old fields!
    oldValue &= 0xFFFF00FF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ( ((fieldMsk & 0x4) == 0x4) && (guestInPrivMode(context)) )
  {
    // status field: reserved and GE[3:0]
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0x00FF0000);
    // clear old fields!
    oldValue &= 0xFF00FFFF;
    // update old value...
    oldValue |= appliedValue;
  }
  if ((fieldMsk & 0x8) == 0x8)
  {
    // condition flags, q, it, J. Dont need to be priv to change those thus no check
    // separate the field we're gonna update from new value 
    u32int appliedValue = (value & 0xFF000000);
    // clear old fields!
    oldValue &= 0x00FFFFFF;
    // update old value...
    oldValue |= appliedValue;
  }
#ifdef ARM_INSTR_TRACE
  serial_putstring("MSR instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
#endif
  // got the final value to write in u32int oldValue. where do we write it thou..?
  if (cpsrOrSpsr == 0)
  {
#ifdef ARM_INSTR_TRACE
    serial_putstring(" value ");
    serial_putint(context->CPSR);
#endif
    // CPSR!
    context->CPSR = oldValue;
  }
  else
  {
    // SPSR! which?... depends what mode we are in...
    switch (context->CPSR & CPSR_MODE_FIELD)
    {
      case CPSR_MODE_FIQ:
        context->SPSR_FIQ = oldValue;
        break;
      case CPSR_MODE_IRQ:
        context->SPSR_IRQ = oldValue;
        break;
      case CPSR_MODE_SVC:
        context->SPSR_SVC = oldValue;
        break;
      case CPSR_MODE_ABORT:
        context->SPSR_ABT = oldValue;
        break;
      case CPSR_MODE_UNDEF:
        context->SPSR_UND = oldValue;
        break;
      case CPSR_MODE_USER:
      case CPSR_MODE_SYSTEM:
      default: 
        DIE_NOW(0, "MSR: invalid SPSR write for current guest mode.");
    } 
  }
#ifdef ARM_INSTR_TRACE
  serial_newline();
#endif
  nextPC = context->PCOfLastInstruction + 4;
  return nextPC;
}

u32int* mrsPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "mrs PCFunct unfinished\n");
  return 0;
}

u32int mrsInstruction(GCONTXT * context)
{
  u32int instr = context->endOfBlockInstr;
  int regSrc   =  instr & 0x00400000;
  int regDest  = (instr & 0x0000F000) >> 12;
  int instrCC = 0;
  u32int value = 0;
  u32int nextPC = 0;
#ifdef ARM_INSTR_TRACE
  serial_putstring("MRS instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->PCOfLastInstruction);
  serial_newline();
#endif
  if (regDest == 0xF)
  {
    invalid_instruction(instr, "MRS cannot use PC as destination");
  }
  instrCC = (instr >> 28) & 0xF;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    if (regSrc == 0)
    {
      // CPSR case
      value = context->CPSR;
    }
    else
    {
      // SPSR case
      int guestMode = (context->CPSR) & CPSR_MODE_FIELD;
      switch(guestMode)
      {
        case CPSR_MODE_FIQ:
          value = context->SPSR_FIQ;
          break;
        case CPSR_MODE_IRQ:
          value = context->SPSR_IRQ;
          break;
        case CPSR_MODE_SVC:
          value = context->SPSR_SVC;
          break;
        case CPSR_MODE_ABORT:
          value = context->SPSR_ABT;
          break;
        case CPSR_MODE_UNDEF:
          value = context->SPSR_UND;
          break;
        case CPSR_MODE_USER:
        case CPSR_MODE_SYSTEM:
        default:
          invalid_instruction(instr, "MRS cannot request spsr in user/system mode");
      } // switch ends
    } // spsr case ends
    storeGuestGPR(regDest, value, context);
  } // condition met ends
  nextPC = context->PCOfLastInstruction + 4;
  return nextPC;
}

u32int* bPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  //Currently a bInstruction is always replaced by an SVC -> do nothing and check for PC in handleFunct
  return 0;
}

u32int bInstruction(GCONTXT * context)
{
  //No register arguments Just change nextPC
  u32int instr = context->endOfBlockInstr;
  u32int instrCC = 0xF0000000 & instr;
  u32int sign = 0x00800000 & instr;
  u32int link = 0x0F000000 & instr;
  int target  = 0x00FFFFFF & instr;
  u32int nextPC = 0;
#ifdef ARM_INSTR_TRACE
  serial_putstring("Branch instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  if (link == 0x0B000000)
  {
    link = 1;
  }
  else
  {
    link = 0;
  }
  // sign extend 24 bit imm to 32 bit offset
  if (sign != 0)
  {
    // target negative!
    target |= 0xFF000000;
  }
  target = target << 2;
  /* eval condition flags */
  instrCC = instrCC >> 28;
  u32int cpsrCC = (context->CPSR >> 28) & 0xF;
  bool conditionMet = evalCC(instrCC, cpsrCC);
  if (conditionMet)
  {
    // condition met
    u32int currPC = context->PCOfLastInstruction;
    currPC += 8;
    nextPC = currPC + target;
    if (link)
    {
      storeGuestGPR(14, context->PCOfLastInstruction+4, context);
    }
  }
  else
  {
    // condition not met!
    nextPC = context->PCOfLastInstruction + 4;
  }
  return nextPC;
}

u32int* svcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "svc PCFunct unfinished\n");
  return 0;
}

u32int svcInstruction(GCONTXT * context)
{
  DIE_NOW(0, "svcInstruction is executed but not yet checked for blockCopyCompatibility");
#ifdef ARM_INSTR_TRACE
  u32int instr = context->endOfBlockInstr;
  serial_putstring("SVC instr ");
  serial_putint(instr);
  serial_putstring(" @ ");
  serial_putint(context->R15);
  serial_newline();
#endif
  return 0;
}

u32int* undefinedPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress)
{
  DIE_NOW(0, "undefined PCFunct unfinished\n");
  return 0;
}

u32int undefinedInstruction(GCONTXT * context)
{
  DIE_NOW(0, "undefinedInstruction is executed but not yet checked for blockCopyCompatibility");
  invalid_instruction(context->endOfBlockInstr, "undefined instruction");
  return 0;
}
