#include "common/debug.h"
#include "common/stddef.h"
#ifdef CONFIG_PROFILER
#include "common/profiler.h"
#endif
 
#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "drivers/beagle/be32kTimer.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beUart.h"

#include "exceptions/exceptionHandlers.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestExceptions.h"
#include "guestManager/scheduler.h"

#include "instructionEmu/blockLinker.h"
#include "instructionEmu/loadStoreDecode.h"
#include "instructionEmu/loopDetector.h"
#include "instructionEmu/scanner.h"
#include "instructionEmu/translator/translator.h"


#include "memoryManager/shadowMap.h"
#include "memoryManager/memoryConstants.h"
#include "memoryManager/memoryProtection.h"

#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/uart.h"

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
#include "instructionEmu/interpreter.h"

static void registerSvc(GCONTXT *context, InstructionHandler handler);
#endif


#ifdef CONFIG_LOOP_DETECTOR
/*
 * We do not care about initializing this variable, because the loop detector is reset during boot.
 * If the initial value would evaluate to TRUE, an extra reset happens at start.
 */
bool mustResetLoopDetector;

static inline void delayResetLoopDetector(void)
{
  mustResetLoopDetector = TRUE;
}

static inline void resetLoopDetectorIfNeeded(GCONTXT *context)
{
  if (mustResetLoopDetector)
  {
    resetLoopDetector(context);
    mustResetLoopDetector = FALSE;
  }
}
#else
#define delayResetLoopDetector()
#define resetLoopDetectorIfNeeded(context)
#endif /* CONFIG_LOOP_DETECTOR */


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code)
{
  bool link = TRUE;
  u32int nextPC = 0;
  bool gSVC = FALSE;

  disableInterrupts();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->svcCount++;
#endif

  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt(%x)" EOL, code);


#ifdef CONFIG_THUMB2
  /* Make sure that any SVC that is not part of the scanner
   * will be delivered to the guest */
  if (context->CPSR & PSR_T_BIT) // Thumb
  {
    if (code == 0) // svc in Thumb is between 0x01 and 0xFF
    {
      DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
      gSVC = TRUE;
    }
  }
  else
  {
    if (code <= 0xFF)
    {
      DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
      gSVC = TRUE;
    }
  }
#else
  if (code < 0x100)
  {
    DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt %#.2x @ %#.8x is a guest system call" EOL, code, context->R15);
    gSVC = TRUE;
  }
#endif

  // Do we need to forward it to the guest?
  if (gSVC)
  {
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
    context->svcGuest++;
#endif
    deliverServiceCall(context);
    nextPC = context->R15;
    link = FALSE;
  }
  else
  {
    u32int cpsrOld = context->CPSR;
    u32int blockStoreIndex = code - 0x100;
    BasicBlock* basicBlock = getBasicBlockStoreEntry(context->translationStore, blockStoreIndex);
    context->lastGuestPC = (u32int)basicBlock->guestEnd;

#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
    registerSvc(context, basicBlock->handler);
#endif

    // interpret the instruction to find the start address of next block
    nextPC = basicBlock->handler(context, *basicBlock->guestEnd);

    u32int cpsrNew = context->CPSR;
    if (((cpsrOld & PSR_MODE) != PSR_USR_MODE) &&
        ((cpsrNew & PSR_MODE) == PSR_USR_MODE))
    {
      // guest was in privileged mode, after interpreting switched to user mode.
      // return from exception, CPS or MSR. act accordingly
      guestToUserMode(context);
    }
    else if (((cpsrOld & PSR_MODE) == PSR_USR_MODE) &&
             ((cpsrNew & PSR_MODE) != PSR_USR_MODE))
    {
      // guest was in user mode. we hit a guest SVC. switch guest to privileged mode
      // return from exception, CPS or MSR. act accordingly
      guestToPrivMode(context);
    }
  }

  if (nextPC == 0)
  {
    DIE_NOW(context, "softwareInterrupt: Invalid nextPC. Instr to implement?");
  }


  traceBlock(context, nextPC);

  u32int lastPC = context->R15;
  context->R15 = nextPC;

  /* Maybe a timer interrupt is pending on real INTC but hasn't been acked yet */
  if (context->guestIrqPending)
  {
    if ((context->CPSR & PSR_I_BIT) == 0)
    {
      deliverInterrupt(context);
      link = FALSE;
    }
  }
  else if (context->guestDataAbtPending)
  {
    deliverDataAbort(context);
    link = FALSE;
  }
 
  DEBUG(EXCEPTION_HANDLERS, "softwareInterrupt: Next PC = %#.8x" EOL, nextPC);

  if ((context->CPSR & PSR_MODE) != PSR_USR_MODE)
  {
    /*
     * guest in privileged mode! scan...
     * We need to reset the loop detector here because the block trace contains 'return' blocks of
     * guest SVCs!
     */
    resetLoopDetectorIfNeeded(context);
    runLoopDetector(context);
    if (context->guestDataAbtPending)
    {
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
    }
    else
    {
      setScanBlockCallSource(SCANNER_CALL_SOURCE_SVC);
    }

    if (link)
    {
      linkBlock(context, context->R15, lastPC, 
                getBasicBlockStoreEntry(context->translationStore, code-0x100));
    }

    scanBlock(context, context->R15);
  }
  else
  {
    // going to user mode.
    delayResetLoopDetector();
  }
  enableInterrupts();
  return context;
}


GCONTXT *dataAbort(GCONTXT *context)
{
  /* Make sure interrupts are disabled while we deal with data abort. */
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->dabtCount++;
  context->dabtUser++;
#endif
  /* Encodings: Page 1289 & 1355 */
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();
  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch (faultStatus)
  {
    case dfsPermissionSection:
    case dfsPermissionPage:
    {
      dabtPermissionFault(context, dfsr, dfar);
      break;
    }
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      dabtTranslationFault(context, dfsr, dfar);
      break;
    }
    case dfsSyncExternalAbt:
    case dfsAlignmentFault:
    case dfsDebugEvent:
    case dfsAccessFlagSection:
    case dfsIcacheMaintenance:
    case dfsAccessFlagPage:
    case dfsDomainSection:
    case dfsDomainPage:
    case dfsTranslationTableWalkLvl1SyncExtAbt:
    case dfsTranslationTableWalkLvl2SyncExtAbt:
    case dfsImpDepLockdown:
    case dfsAsyncExternalAbt:
    case dfsMemAccessAsyncParityErr:
    case dfsMemAccessAsyncParityERr2:
    case dfsImpDepCoprocessorAbort:
    case dfsTranslationTableWalkLvl1SyncParityErr:
    case dfsTranslationTableWalkLvl2SyncParityErr:
    default:
    {
      printf("unimplemented user data abort %#.8x" EOL, faultStatus);
      printDataAbort();
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
  return context;
}


void dataAbortPrivileged(u32int pc, u32int sp, u32int spsr)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->dabtCount++;
  getActiveGuestContext()->dabtPriv++;
#endif
  printf("dabtPriv @ %08x\n", pc);
  u32int dfar = getDFAR();
  DFSR dfsr = getDFSR();

  u32int faultStatus = (dfsr.fs3_0) | (dfsr.fs4 << 4);
  switch(faultStatus)
  {
    case dfsTranslationSection:
    case dfsTranslationPage:
    {
      dabtTranslationFault(getActiveGuestContext(), dfsr, dfar);
      break;
    }
    case dfsAlignmentFault:
    case dfsDebugEvent:
    case dfsAccessFlagSection:
    case dfsIcacheMaintenance:
    case dfsAccessFlagPage:
    case dfsSyncExternalAbt:
    case dfsDomainSection:
    case dfsDomainPage:
    case dfsTranslationTableWalkLvl1SyncExtAbt:
    case dfsPermissionSection:
    case dfsTranslationTableWalkLvl2SyncExtAbt:
    case dfsPermissionPage:
    case dfsImpDepLockdown:
    case dfsAsyncExternalAbt:
    case dfsMemAccessAsyncParityErr:
    case dfsMemAccessAsyncParityERr2:
    case dfsImpDepCoprocessorAbort:
    case dfsTranslationTableWalkLvl1SyncParityErr:
    case dfsTranslationTableWalkLvl2SyncParityErr:
    default:
    {
      printf("dataAbortPrivileged pc %08x addr %08x" EOL, pc, dfar);
      printDataAbort();
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
  }
}


GCONTXT *undefined(GCONTXT *context)
{
  DIE_NOW(context, "undefined: undefined handler, Implement me!");
}


void undefinedPrivileged(void)
{
  DIE_NOW(NULL, "undefinedPrivileged: Undefined handler, privileged mode. Implement me!");
}


GCONTXT *prefetchAbort(GCONTXT *context)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->pabtCount++;
  context->pabtUser++;
#endif
  // Make sure interrupts are disabled while we deal with prefetch abort.
  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);

  switch(faultStatus)
  {
    case ifsTranslationFaultSection:
    case ifsTranslationFaultPage:
    {
      iabtTranslationFault(context, ifsr, ifar);
      break;
    }
    case ifsTranslationTableWalk2ndLvllSynchExtAbt:
#ifdef CONFIG_GUEST_FREERTOS
    {
      if (shouldPrefetchAbort(context, isGuestInPrivMode(context), ifar))
      {
        deliverPrefetchAbort(context);
        setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_FREERTOS);
        scanBlock(context, context->R15);
      }
      break;
    }
#endif
    case ifsDebugEvent:
    case ifsAccessFlagFaultSection:
    case ifsAccessFlagFaultPage:
    case ifsSynchronousExternalAbort:
    case ifsDomainFaultSection:
    case ifsDomainFaultPage:
    case ifsTranslationTableTalk1stLvlSynchExtAbt:
    case ifsPermissionFaultSection:
    case ifsPermissionFaultPage:
    case ifsImpDepLockdown:
    case ifsMemoryAccessSynchParityError:
    case ifsImpDepCoprocessorAbort:
    case ifsTranslationTableWalk1stLvlSynchParityError:
    case ifsTranslationTableWalk2ndLvlSynchParityError:
    default:
    {
      printPrefetchAbort();
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }
  return context;
}


void prefetchAbortPrivileged(void)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->pabtCount++;
  getActiveGuestContext()->pabtPriv++;
#endif

  IFSR ifsr = getIFSR();
  u32int faultStatus = (ifsr.fs3_0) | (ifsr.fs4 << 4);
  switch(faultStatus)
  {
    case ifsTranslationFaultPage:
    case ifsDebugEvent:
    case ifsAccessFlagFaultSection:
    case ifsTranslationFaultSection:
    case ifsAccessFlagFaultPage:
    case ifsSynchronousExternalAbort:
    case ifsDomainFaultSection:
    case ifsDomainFaultPage:
    case ifsTranslationTableTalk1stLvlSynchExtAbt:
    case ifsPermissionFaultSection:
    case ifsTranslationTableWalk2ndLvllSynchExtAbt:
    case ifsPermissionFaultPage:
    case ifsImpDepLockdown:
    case ifsMemoryAccessSynchParityError:
    case ifsImpDepCoprocessorAbort:
    case ifsTranslationTableWalk1stLvlSynchParityError:
    case ifsTranslationTableWalk2ndLvlSynchParityError:
    default:
    {
      printPrefetchAbort();
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}


GCONTXT *monitorMode(GCONTXT *context)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
}


void monitorModePrivileged(void)
{
  /*
   * TODO
   * Does the omap 3 implement monitor/secure mode?
   * Niels: yes it does!
   */
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}


GCONTXT *irq(GCONTXT *context)
{
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  getActiveGuestContext()->irqCount++;
  getActiveGuestContext()->irqUser++;
#endif

  // Get the number of the highest priority active IRQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
    {
      scheduleGuest();
      gptBEClearOverflowInterrupt(1);
      acknowledgeIrqBE();
      break;
    }
    case GPT2_IRQ:
    {
      throwInterrupt(context, activeIrqNumber);
      BasicBlock* block = getBasicBlockStoreEntry(context->translationStore, context->lastEntryBlockIndex);
      // now we must unlink the current block if it is in a group block.
      // to make sure the guest isn't waiting for our deferred interrupt forever
      if (block->type == GB_TYPE_ARM)
      {
        u32int index = findBlockIndexNumber(context, context->R15);
        BasicBlock* groupblock = getBasicBlockStoreEntry(context->translationStore, index);
        unlinkBlock(groupblock, index);
      }
      
      // FIXME: figure out which interrupt to clear and then clear the right one?
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(2);
        gptBEClearMatchInterrupt(2);
      }
#endif
      acknowledgeIrqBE();
      break;
    }
    case UART3_IRQ:
    {
      /*
       * FIXME: Niels: are we sure we're supposed to read characters unconditionally?
       */
      // read character from UART
      u8int c = serialGetcAsync();
      acknowledgeIrqBE();
      // forward character to emulated UART
      uartPutRxByte(context, c, 3);
      break;
    }
    default:
    {
      printf("Received IRQ = %x" EOL, activeIrqNumber);
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
    }
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQss,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ line is de-asserted before IRQ enabling. */
  __asm__ __volatile__("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
  return context;
}


void irqPrivileged()
{
  GCONTXT *const context = getActiveGuestContext();
#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
  context->irqCount++;
  context->irqPriv++;
#endif

  // Get the number of the highest priority active IRQ
  u32int activeIrqNumber = getIrqNumberBE();
  switch(activeIrqNumber)
  {
    case GPT1_IRQ:
    {
      gptBEClearOverflowInterrupt(1);
      acknowledgeIrqBE();
      break;
    }
    case GPT2_IRQ:
    {
      throwInterrupt(context, activeIrqNumber);
      /*
       * FIXME: figure out which interrupt to clear and then clear the right one?
       */
      gptBEClearOverflowInterrupt(2);
#ifdef CONFIG_GUEST_FREERTOS
      if (context->os == GUEST_OS_FREERTOS)
      {
        gptBEResetCounter(2);
        gptBEClearMatchInterrupt(2);
      }
#endif
      acknowledgeIrqBE();
      break;
    }
    case UART3_IRQ:
    {
      /*
       * FIXME: Niels: are we sure we're supposed to read characters unconditionally?
       */
      // read character from UART
      u8int c = serialGetcAsync();
      acknowledgeIrqBE();
      // forward character to emulated UART
      uartPutRxByte(context, c, 3);
      break;
    }
    default:
    {
      printf("Received IRQ = %#x" EOL, activeIrqNumber);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }

  /* Because the writes are posted on an Interconnect bus, to be sure
   * that the preceding writes are done before enabling IRQss,
   * a Data Synchronization Barrier is used. This operation ensure that
   * the IRQ line is de-asserted before IRQ enabling. */
  __asm__ __volatile__("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
}


void fiq(u32int addr)
{
#ifdef CONFIG_PROFILER
//  printf("-");
  GCONTXT *const context = getActiveGuestContext();
  
  u32int activeFiqNumber = getFiqNumberBE();
  switch (activeFiqNumber)
  {
    case GPT3_IRQ:
    {
//      u32int pAddr = getPhysicalAddress(context, context->pageTables->shadowActive, addr);
      profilerRecord(addr);
      gptBEClearOverflowInterrupt(3);
      break;
    }
    default:
      DIE_NOW(NULL, "unimplemented FIQ handler.");
  }
  
  acknowledgeFiqBE();
  
  // write barrier
  asm volatile("MOV R0, #0\n\t"
               "MCR P15, #0, R0, C7, C10, #4"
               : : : "memory");
//  printf(".");
#else
   DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
 }


void dabtPermissionFault(GCONTXT *gc, DFSR dfsr, u32int dfar)
{
  // Check if the addr we have faulted on is caused by
  // a memory protection the hypervisor has enabled
  if (gc->virtAddrEnabled)
  {
    if (shouldDataAbort(gc, isGuestInPrivMode(gc), dfsr.WnR, dfar))
    {
      deliverDataAbort(gc);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION);
      scanBlock(gc, gc->R15);
      return;
    }
  }

  // interpret the load/store
  emulateLoadStoreGeneric(gc, dfar);
  
  // load/store might still have failed if it was LDRT/STRT
  if (!gc->guestDataAbtPending)
  {
    // ONLY move to the next instruction, if the guest hasn't aborted...
#ifdef CONFIG_THUMB2
    if (gc->CPSR & PSR_T_BIT)
    {
      gc->R15 = gc->R15 + T16_INSTRUCTION_SIZE;
    }
    else
#endif
    {
      gc->R15 = gc->R15 + ARM_INSTRUCTION_SIZE;
    }
  }
  else
  {
    // deliver the abort!
    deliverDataAbort(gc);
    setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_PERMISSION);
    scanBlock(gc, gc->R15);
  }
}


void dabtTranslationFault(GCONTXT *context, DFSR dfsr, u32int dfar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
  if (!shadowMap(context, dfar))
  {
    // failed to shadow map!
    if (shouldDataAbort(context, isGuestInPrivMode(context), dfsr.WnR, dfar))
    {
      if (isGuestInPrivMode(context))
      {
        // we need to map host PC to guest PC
        context->R15 = hostpcToGuestpc(context);
        DEBUG(EXCEPTION_HANDLERS, "dabtTranslationFault: mapping PC got %08x\n", context->R15);
      }
      /*
       * here we must find the correct guest PC to store in R14_ABT
       * - if guest was running in user mode
       *   then code was executed in place, and context has the correct gPC
       * - if guest was privileged mode, guest code was executed from code store
       *   then we must find correct guest PC
       */
      deliverDataAbort(context);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_DABT_TRANSLATION);
      scanBlock(context, context->R15);
      return;
    }
    else
    {
      DIE_NOW(NULL, "dabtTranslationFault: panic now!\n");
    }
  }
}


void iabtTranslationFault(GCONTXT *context, IFSR ifsr, u32int ifar)
{
  /* if we hit this - that means that:
     mem access address corresponding 1st level page table entry FAULT/reserved
     or corresponding 2nd level page table entry FAULT
     see if translation fault should be forwarded to the guest!
     if THAT fails, then really panic.
   */
  if (!shadowMap(context, ifar))
  {
    // failed to shadow map!
    if (shouldPrefetchAbort(context, isGuestInPrivMode(context), ifar))
    {
      if (isGuestInPrivMode(context))
      {
        // we need to map host PC to guest PC
        context->R15 = hostpcToGuestpc(context);
        DEBUG(EXCEPTION_HANDLERS, "dabtTranslationFault: mapping PC got %08x\n", context->R15);
      }
      deliverPrefetchAbort(context);
      setScanBlockCallSource(SCANNER_CALL_SOURCE_PABT_TRANSLATION);
      scanBlock(context, context->R15);
      return;
    }
    else
    {
      DIE_NOW(context, "iabtTranslationFault: panic now!\n");
    }
  }
}


#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
void registerSvc(GCONTXT *context, InstructionHandler handler)
{
  if (handler == armStmInstruction)
  {
    context->armStmInstruction++;
  }
  else if (handler == armLdmInstruction)
  {
    context->armLdmInstruction++;
  }
  else if (handler == armLdrInstruction)
  {
    context->armLdrInstruction++;
  }
  else if (handler == armBInstruction)
  {
    context->armBInstruction++;
  }
  else if (handler == armSwpInstruction)
  {
    context->armSwpInstruction++;
  }
  else if (handler == armLdrexbInstruction)
  {
    context->armLdrexbInstruction++;
  }
  else if (handler == armLdrexdInstruction)
  {
    context->armLdrexdInstruction++;
  }
  else if (handler == armLdrexhInstruction)
  {
    context->armLdrexhInstruction++;
  }
  else if (handler == armStrexbInstruction)
  {
    context->armStrexbInstruction++;
  }
  else if (handler == armStrexdInstruction)
  {
    context->armStrexdInstruction++;
  }
  else if (handler == armStrexhInstruction)
  {
    context->armStrexhInstruction++;
  }
  else if (handler == armLdrexInstruction)
  {
    context->armLdrexInstruction++;
  }
  else if (handler == armStrexInstruction)
  {
    context->armStrexInstruction++;
  }
  else if (handler == armBxInstruction)
  {
    context->armBxInstruction++;
  }
  else if (handler == armBxjInstruction)
  {
    context->armBxjInstruction++;
  }
  else if (handler == armBkptInstruction)
  {
    context->armBkptInstruction++;
  }
  else if (handler == armSmcInstruction)
  {
    context->armSmcInstruction++;
  }
  else if (handler == armBlxRegisterInstruction)
  {
    context->armBlxRegisterInstruction++;
  }
  else if (handler == armAndInstruction)
  {
    context->armAndInstruction++;
  }
  else if (handler == armEorInstruction)
  {
    context->armEorInstruction++;
  }
  else if (handler == armSubInstruction)
  {
    context->armSubInstruction++;
  }
  else if (handler == armAddInstruction)
  {
    context->armAddInstruction++;
  }
  else if (handler == armAdcInstruction)
  {
    context->armAdcInstruction++;
  }
  else if (handler == armSbcInstruction)
  {
    context->armSbcInstruction++;
  }
  else if (handler == armRscInstruction)
  {
    context->armRscInstruction++;
  }
  else if (handler == armMsrInstruction)
  {
    context->armMsrInstruction++;
  }
  else if (handler == armMrsInstruction)
  {
    context->armMrsInstruction++;
  }
  else if (handler == armOrrInstruction)
  {
    context->armOrrInstruction++;
  }
  else if (handler == armMovInstruction)
  {
    context->armMovInstruction++;
  }
  else if (handler == armLslInstruction)
  {
    context->armLslInstruction++;
  }
  else if (handler == armLsrInstruction)
  {
    context->armLsrInstruction++;
  }
  else if (handler == armAsrInstruction)
  {
    context->armAsrInstruction++;
  }
  else if (handler == armRrxInstruction)
  {
    context->armRrxInstruction++;
  }
  else if (handler == armRorInstruction)
  {
    context->armRorInstruction++;
  }
  else if (handler == armBicInstruction)
  {
    context->armBicInstruction++;
  }
  else if (handler == armMvnInstruction)
  {
    context->armMvnInstruction++;
  }
  else if (handler == armYieldInstruction)
  {
    context->armYieldInstruction++;
  }
  else if (handler == armWfeInstruction)
  {
    context->armWfeInstruction++;
  }
  else if (handler == armWfiInstruction)
  {
    context->armWfiInstruction++;
  }
  else if (handler == armSevInstruction)
  {
    context->armSevInstruction++;
  }
  else if (handler == armDbgInstruction)
  {
    context->armDbgInstruction++;
  }
  else if (handler == svcInstruction)
  {
    context->svcInstruction++;
  }
  else if (handler == armMrcInstruction)
  {
    context->armMrcInstruction++;
  }
  else if (handler == armMcrInstruction)
  {
    context->armMcrInstruction++;
  }
  else if (handler == armDmbInstruction)
  {
    context->armDmbInstruction++;
  }
  else if (handler == armDsbInstruction)
  {
    context->armDsbInstruction++;
  }
  else if (handler == armIsbInstruction)
  {
    context->armIsbInstruction++;
  }
  else if (handler == armClrexInstruction)
  {
    context->armClrexInstruction++;
  }
  else if (handler == armCpsInstruction)
  {
    context->armCpsInstruction++;
  }
  else if (handler == armRfeInstruction)
  {
    context->armRfeInstruction++;
  }
  else if (handler == armSetendInstruction)
  {
    context->armSetendInstruction++;
  }
  else if (handler == armSrsInstruction)
  {
    context->armSrsInstruction++;
  }
  else if (handler == armBlxImmediateInstruction)
  {
    context->armBlxImmediateInstruction++;
  }
  else if (handler == armPldInstruction)
  {
    context->armPldInstruction++;
  }
  else if (handler == armPliInstruction)
  {
    context->armPliInstruction++;
  }
  else if (handler == armStrbtInstruction)
  {
    context->armStrbtInstruction++;
  }
  else if (handler == armStrhtInstruction)
  {
    context->armStrhtInstruction++;
  }
  else if (handler == armStrtInstruction)
  {
    context->armStrtInstruction++;
  }
  else if (handler == armLdrbtInstruction)
  {
    context->armLdrbtInstruction++;
  }
  else if (handler == armLdrhtInstruction)
  {
    context->armLdrhtInstruction++;
  }
  else if (handler == armLdrtInstruction)
  {
    context->armLdrtInstruction++;
  }
  else
  {
    printf("handler = %p", handler);
    DIE_NOW(context, "handler...");
  }
}
#endif
