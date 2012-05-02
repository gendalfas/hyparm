#ifndef __EXCEPTIONS__EXCEPTION_HANDLERS_H__
#define __EXCEPTIONS__EXCEPTION_HANDLERS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/mmu.h"

#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_DATA_ABORT
u64int getDataAbortCounter(void);
void resetDataAbortCounter(void);
#else
#define getDataAbortCounter()    (0ULL)
#define resetDataAbortCounter()
#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_DATA_ABORT */

#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_IRQ
u64int getIrqCounter(void);
void resetIrqCounter(void);
#else
#define getIrqCounter()    (0ULL)
#define resetIrqCounter()
#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_IRQ */


#ifdef CONFIG_EXCEPTION_HANDLERS_COUNT_SVC
u64int getSvcCounter(void);
void resetSvcCounter(void);
#else
#define getSvcCounter()    (0ULL)
#define resetSvcCounter()
#endif /* CONFIG_EXCEPTION_HANDLERS_COUNT_SVC */


GCONTXT *softwareInterrupt(GCONTXT *context, u32int code);

GCONTXT *dataAbort(GCONTXT *context);
void dataAbortPrivileged(u32int pc, u32int sp, u32int spsr);
//void dataAbortPrivileged(u32int pc, u32int sp);

GCONTXT *undefined(GCONTXT *context);
void undefinedPrivileged(void);

GCONTXT *prefetchAbort(GCONTXT *context);
void prefetchAbortPrivileged(void);

GCONTXT *monitorMode(GCONTXT *context);
void monitorModePrivileged(void);

GCONTXT *irq(GCONTXT *context);
void irqPrivileged(void);

void fiq(void);

void dabtTranslationFault(GCONTXT * gc, DFSR dfsr, u32int dfar);
void dabtPermissionFault(GCONTXT * gc, DFSR dfsr, u32int dfar);

void iabtTranslationFault(GCONTXT * gc, IFSR ifsr, u32int ifar);
#endif

